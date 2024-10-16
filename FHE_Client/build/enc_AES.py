from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import padding
import os
import json
import time

def generate_key():
    return os.urandom(32)  # Generates a random 256-bit key

def encrypt_file(input_file, output_file, key, iv):
    backend = default_backend()
    cipher = Cipher(algorithms.AES(key), modes.CBC(iv), backend=backend)
    encryptor = cipher.encryptor()
    padder = padding.PKCS7(algorithms.AES.block_size).padder()

    with open(input_file, 'rb') as f_in:
        data = f_in.read()
        padded_data = padder.update(data) + padder.finalize()
        encrypted_data = encryptor.update(padded_data) + encryptor.finalize()

    with open(output_file, 'wb') as f_out:
        f_out.write(iv + encrypted_data)  # Prepend IV to encrypted data

if __name__ == "__main__":
    key = generate_key()
    key_dir = 'Keys'
    encrypted_dir = 'Encrypted'
    key_info = {}

    # Create Encrypted directory if it doesn't exist
    if not os.path.exists(encrypted_dir):
        os.makedirs(encrypted_dir)

    # Encrypt all files in the Key directory
    for filename in os.listdir(key_dir):
        if os.path.isfile(os.path.join(key_dir, filename)):
            iv = os.urandom(16)
            input_file = os.path.join(key_dir, filename)
            encrypted_file = os.path.join(encrypted_dir, f"{filename}.enc")

            encrypt_file(input_file, encrypted_file, key, iv)
            print(f"File '{input_file}' has been encrypted to '{encrypted_file}'.")

            # Store the IV for each file
            key_info[filename] = iv.hex()

    # Save the key and IVs
    with open('encryption_info.json', 'w') as f:
        json.dump({'key': key.hex(), 'ivs': key_info}, f)

    print("Encryption complete. Key and IVs saved to 'encryption_info.json'.")
