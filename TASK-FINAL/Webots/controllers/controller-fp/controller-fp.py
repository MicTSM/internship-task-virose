from controller import Robot, Motor, PositionSensor, Keyboard
import json
import os
import sys
import time

SERIAL_PORT_NAME = "COM3"  
SERIAL_BAUD_RATE = 115200

# Cek apakah modul pyserial tersedia untuk komunikasi serial
USE_SERIAL = False
serial_module = None
try:
    import serial
    import serial.tools.list_ports
    USE_SERIAL = True
    serial_module = serial
    print("[INFO] pyserial module loaded successfully")
except ImportError:
    print("[WARNING] pyserial not installed, using keyboard mode only")
    print("[INFO] Install with: pip install pyserial")

# Daftar 20 motor robot yang mengontrol berbagai bagian tubuh
# Head/Neck: kepala dan leher, Shoulder/Arm: bahu dan lengan, Pelv: panggul
# Leg/LegLower: paha dan betis, Ankle/Foot: pergelangan kaki dan kaki
MOTOR_NAMES = [
    "Head", "Neck",
    "ShoulderL", "ShoulderR",
    "ArmUpperL", "ArmUpperR",
    "ArmLowerL", "ArmLowerR",
    "PelvYL", "PelvYR",
    "PelvL", "PelvR",
    "LegUpperL", "LegUpperR",
    "LegLowerL", "LegLowerR",
    "AnkleL", "AnkleR",
    "FootL", "FootR"
]

NUM_MOTORS = len(MOTOR_NAMES)

# Inisialisasi robot Webots dan timestep untuk simulasi
robot = Robot()
timeStep = int(robot.getBasicTimeStep())

# Inisialisasi semua motor dan position sensor
# Position sensor digunakan untuk membaca posisi saat ini dari setiap motor
motors = {}
sensors = {}

print("Initializing motors...")
for name in MOTOR_NAMES:
    motor = robot.getMotor(name)
    if motor:
        motors[name] = motor
        # Position sensor memiliki nama motor + "S" (misalnya HeadS untuk motor Head)
        sensor = robot.getPositionSensor(name + "S")
        if sensor:
            sensor.enable(timeStep)
            sensors[name] = sensor

# Inisialisasi koneksi serial dengan ESP32 Receiver
# Alur komunikasi: Laptop 2 -> ESP32 Transmitter -> ESP-NOW -> ESP32 Receiver -> USB Serial -> Controller ini
# ESP32 Receiver mengirimkan karakter (W, S, A, D, Q, E, F) melalui serial USB
serial_port = None
if USE_SERIAL:
    # Scan dan tampilkan semua port COM yang tersedia untuk debugging
    print("\n[INFO] Scanning for available COM ports...")
    available_ports = []
    try:
        ports = serial.tools.list_ports.comports()
        for port in ports:
            available_ports.append(port.device)
            print(f"  - {port.device}: {port.description}")
    except Exception as e:
        print(f"[WARNING] Could not list ports: {e}")
    
    # Coba buka koneksi serial dengan ESP32 Receiver
    try:
        print(f"\n[INFO] Attempting to open serial port: {SERIAL_PORT_NAME}")
        serial_port = serial.Serial(
            port=SERIAL_PORT_NAME,
            baudrate=SERIAL_BAUD_RATE,
            timeout=0.1,
            write_timeout=0.1,
            dsrdtr=False,
            rtscts=False,
            xonxoff=False
        )
        # Tunggu koneksi stabil dan bersihkan buffer dari data lama
        time.sleep(0.5)
        serial_port.reset_input_buffer()
        serial_port.reset_output_buffer()
        
        print(f"[OK] Serial port {SERIAL_PORT_NAME} opened successfully at {SERIAL_BAUD_RATE} baud")
        print(f"[INFO] Connection stabilized, buffer cleared")
        print(f"[INFO] Listening for commands from ESP32 Receiver...")
        print(f"[INFO] Expected keys: W, S, A, D, Q, E, F")
    except serial.SerialException as e:
        print(f"[ERROR] Cannot open serial port {SERIAL_PORT_NAME}: {e}")
        if available_ports:
            print(f"[INFO] Available ports: {', '.join(available_ports)}")
            print(f"[INFO] Please update SERIAL_PORT_NAME in the code if needed")
        print("[INFO] Falling back to keyboard mode")
        serial_port = None
    except Exception as e:
        print(f"[ERROR] Unexpected error opening serial port: {e}")
        print("[INFO] Falling back to keyboard mode")
        serial_port = None

# Inisialisasi keyboard input untuk testing lokal
# Keyboard tetap bisa digunakan bahkan jika serial sudah terhubung
keyboard = robot.getKeyboard()
keyboard.enable(timeStep)
if serial_port:
    print("[INFO] Keyboard input also available for local testing")
else:
    print("[INFO] Using Keyboard input mode (serial not available)")

# Fungsi untuk menemukan folder poses dengan beberapa fallback
# Cek environment variable, absolute path, atau relative path
def get_poses_folder():
    """Auto-detect poses folder with multiple fallbacks"""
    env_path = os.getenv("WEBOTS_POSES_PATH")
    if env_path and os.path.exists(env_path):
        return env_path if env_path.endswith(os.sep) else env_path + os.sep
    
    abs_path = "C:/Users/frost/Robotik/TASK-FINAL/Webots/poses/"
    if os.path.exists(abs_path):
        return abs_path
    
    # Coba relative paths dari berbagai lokasi controller
    relative_paths = ["../../poses/", "../poses/", "poses/"]
    for path in relative_paths:
        if os.path.exists(path):
            return path
    
    return abs_path  # Default fallback

# Fungsi untuk memuat dan parse file JSON pose
# File JSON berisi posisi target untuk semua motor dalam berbagai pose
def load_pose_file(filepath):
    """Load and parse pose JSON file"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            data = json.load(f)
        return data
    except Exception as e:
        print(f"[ERROR] Cannot load {filepath}: {e}")
        return None

# Fungsi interpolasi halus untuk gerakan motor menggunakan interpolasi linear
# Motor bergerak secara bertahap ke posisi target, bukan langsung lompat
# Parameter: target_positions (list posisi target), steps (jumlah langkah interpolasi)
def set_motor_positions(target_positions, steps=30):
    """Smoothly interpolate motors to target positions"""
    # Baca posisi saat ini dari semua position sensor
    current_positions = []
    for name in MOTOR_NAMES:
        if name in sensors:
            current_positions.append(sensors[name].getValue())
        else:
            current_positions.append(0.0)
    
    # Interpolasi linear: hitung posisi intermediate untuk setiap step
    for step in range(1, steps + 1):
        fraction = step / float(steps)  # Fraksi dari 0.0 sampai 1.0
        for i, name in enumerate(MOTOR_NAMES):
            if name in motors and i < len(target_positions):
                # Formula: posisi_baru = posisi_awal + (target - awal) * fraksi
                new_pos = current_positions[i] + (target_positions[i] - current_positions[i]) * fraction
                motors[name].setPosition(new_pos)
        robot.step(timeStep)  # Jalankan satu step simulasi Webots

# Fungsi untuk mengeksekusi urutan pose dari file JSON
# File JSON berisi pose_group, setiap group berisi beberapa pose yang dieksekusi berurutan
# Setelah setiap group selesai, ada jeda kecil agar gerakan lebih natural
def execute_pose(pose_data):
    """Execute all pose groups and poses in sequence"""
    if not pose_data or 'pose_group' not in pose_data:
        return
    
    # Eksekusi setiap pose_group dan pose di dalamnya secara berurutan
    for group in pose_data['pose_group']:
        for pose in group['pose']:
            # Validasi: pastikan pose memiliki 20 posisi motor
            if 'posisi' in pose and len(pose['posisi']) == NUM_MOTORS:
                set_motor_positions(pose['posisi'], steps=30)
        # Jeda antar group agar gerakan lebih natural
        for _ in range(10):
            robot.step(timeStep)

pose_folder = get_poses_folder()
print(f"\nLoading poses from: {pose_folder}")

pose_files = [
    "pose-jongkok",
    "pose-berdiri",
    "pose-jalan-maju",
    "pose-jalan-mundur",
    "pose-geser-kanan",
    "pose-geser-kiri",
    "pose-belok-kanan",
    "pose-belok-kiri"
]

pose_data_dict = {}
for fname in pose_files:
    filepath = os.path.join(pose_folder, fname + ".json")
    data = load_pose_file(filepath)
    if data:
        pose_data_dict[fname] = data
        print(f"[OK] {fname}")
    else:
        print(f"[ERROR] {fname}")

print(f"\nLoaded: {len(pose_data_dict)} / {len(pose_files)}")

# Set pose awal: robot langsung dalam posisi berdiri saat simulasi dimulai
# Ini mencegah robot jatuh atau dalam posisi yang tidak wajar
if "pose-berdiri" in pose_data_dict:
    berdiri = pose_data_dict["pose-berdiri"]
    if berdiri['pose_group'] and berdiri['pose_group'][0]['pose']:
        initial_pose = berdiri['pose_group'][0]['pose'][0]['posisi']
        print("\nSetting initial standing pose...")
        # Set semua motor ke posisi berdiri
        for i, name in enumerate(MOTOR_NAMES):
            if name in motors and i < len(initial_pose):
                motors[name].setPosition(initial_pose[i])
        # Jalankan beberapa step agar robot mencapai posisi berdiri
        for _ in range(100):
            robot.step(timeStep)
        print("[OK] Initial pose set\n")

# Pemetaan tombol keyboard ke pose robot
# W: maju, S: mundur, A: geser kiri, D: geser kanan
# Q: belok kiri, E: belok kanan, F: jongkok
# Karakter ini diterima dari keyboard atau ESP32 Receiver via serial
KEY_MAP = {
    ord('W'): "pose-jalan-maju",
    ord('S'): "pose-jalan-mundur",
    ord('A'): "pose-geser-kiri",
    ord('D'): "pose-geser-kanan",
    ord('Q'): "pose-belok-kiri",
    ord('E'): "pose-belok-kanan",
    ord('F'): "pose-jongkok"
}

print("========================================")
print("Robot Controller Ready!")
if serial_port:
    print(f"[MODE] Serial Input Active (Port: {SERIAL_PORT_NAME})")
    print("[MODE] Keyboard Input Also Available (for local testing)")
    print("\n[PRIORITY] Serial commands take priority over keyboard")
    print("Waiting for commands from ESP32 Receiver...")
    print("\n[FLOW] Laptop 2 -> ESP32 Transmitter -> ESP-NOW -> ESP32 Receiver -> Serial USB -> This Controller")
    print("[INFO] Make sure ESP32 Receiver is powered and connected to COM3")
    print("[INFO] When tester presses keys on Laptop 2, you should see [SERIAL] messages here")
    print("[INFO] You can also use keyboard (W-S-A-D-Q-E-F) for local testing")
else:
    print("[MODE] Keyboard Input Only")
    print("W-S-A-D-Q-E-F keys to control")
    print("\n[INFO] Serial port not available - using keyboard mode only")
    print("[INFO] If you want to use ESP32 Receiver, check:")
    print("  1. COM port is correct (currently set to: " + SERIAL_PORT_NAME + ")")
    print("  2. ESP32 Receiver is connected and powered")
    print("  3. pyserial is installed: pip install pyserial")
print("========================================\n")

# Loop kontrol utama - berjalan terus selama simulasi aktif
# executing: flag untuk mencegah input baru saat pose sedang dieksekusi
# debounce_time: waktu minimum antar input (0.5 detik) untuk mencegah spam
executing = False
last_key_time = 0.0
debounce_time = 0.5

while robot.step(timeStep) != -1:
    # Hanya proses input jika tidak sedang mengeksekusi pose
    if not executing:
        key = -1
        
        # Baca input dari serial USB (prioritas tinggi)
        # ESP32 Receiver mengirimkan karakter tunggal melalui serial
        if serial_port:
            try:
                if serial_port.in_waiting > 0:
                    # Baca 1 byte dari serial buffer
                    byte_data = serial_port.read(1)
                    if byte_data and len(byte_data) > 0:
                        key = ord(byte_data)  # Konversi byte ke ASCII
                        if key in KEY_MAP:
                            print(f"[SERIAL] Received key: '{chr(key)}' (ASCII: {key}) -> {KEY_MAP[key]}")
                        elif 32 <= key <= 126:
                            print(f"[SERIAL] Received key: '{chr(key)}' (ASCII: {key}) - not in KEY_MAP, ignored")
                        else:
                            print(f"[SERIAL] Received non-printable: ASCII {key} (0x{key:02X}) - ignored")
            except Exception as e:
                # Tangani error komunikasi serial, fallback ke keyboard mode
                error_type = type(e).__name__
                if 'SerialException' in error_type or 'Serial' in error_type:
                    print(f"[ERROR] Serial communication error: {e}")
                    print("[INFO] Serial port may have been disconnected")
                    try:
                        serial_port.close()
                    except:
                        pass
                    serial_port = None
                    print("[INFO] Switched to keyboard mode")
                else:
                    print(f"[ERROR] Serial read error: {e}")
        
        # Baca input dari keyboard jika tidak ada input serial (untuk testing lokal)
        if key == -1:
            key = keyboard.getKey()
            if key != -1 and key in KEY_MAP:
                print(f"[KEYBOARD] Key pressed: '{chr(key)}' (ASCII: {key}) -> {KEY_MAP[key]}")
        
        # Eksekusi pose berdasarkan tombol yang ditekan
        # Gunakan debounce untuk mencegah spam input
        if key != -1 and key in KEY_MAP:
            current_time = robot.getTime()
            # Debounce: hanya terima input jika sudah lewat waktu minimum
            if current_time - last_key_time > debounce_time:
                pose_name = KEY_MAP[key]
                if pose_name in pose_data_dict:
                    executing = True  # Lock input selama eksekusi pose
                    last_key_time = current_time
                    execute_pose(pose_data_dict[pose_name])
                    executing = False  # Unlock setelah selesai

if serial_port:
    serial_port.close()
