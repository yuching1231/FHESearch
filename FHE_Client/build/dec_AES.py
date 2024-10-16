from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import padding
import os
import json
import time

def decrypt_file(input_file, output_file, key, iv):
    backend = default_backend()
    cipher = Cipher(algorithms.AES(key), modes.CBC(iv), backend=backend)
    decryptor = cipher.decryptor()
    unpadder = padding.PKCS7(algorithms.AES.block_size).unpadder()

    with open(input_file, 'rb') as f_in:
        encrypted_data = f_in.read()
        decrypted_data = decryptor.update(encrypted_data) + decryptor.finalize()
        unpadded_data = unpadder.update(decrypted_data) + unpadder.finalize()

    with open(output_file, 'wb') as f_out:
        f_out.write(unpadded_data)

if __name__ == "__main__":
    encrypted_dir = 'Encrypted'
    decrypted_dir = 'Decrypted'

    # Create Decrypted directory if it doesn't exist
    if not os.path.exists(decrypted_dir):
        os.makedirs(decrypted_dir)

    # Load the key and IVs
    with open('encryption_info.json', 'r') as f:
        encryption_info = json.load(f)

    key = bytes.fromhex(encryption_info['key'])

    # Decrypt all files in the Encrypted directory
    for filename in os.listdir(encrypted_dir):
        if filename.endswith('.enc'):
            original_filename = filename[:-4]  # Remove .enc extension
            iv = bytes.fromhex(encryption_info['ivs'][original_filename])

            input_file = os.path.join(encrypted_dir, filename)
            decrypted_file = os.path.join(decrypted_dir, original_filename)

            decrypt_file(input_file, decrypted_file, key, iv)
            print(f"File '{input_file}' has been decrypted to '{decrypted_file}'.")

    print("Decryption complete.")
