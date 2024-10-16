const { exec } = require('child_process');
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const { ipcRenderer } = require('electron');

console.log('local.js loaded');

function addClickListener(id, message) {
    const element = document.getElementById(id);
    if (element) {
        element.addEventListener('click', () => {
            console.log(message);
            document.getElementById('status').textContent = message;
        });
    } else {
        console.error(`Element with id "${id}" not found`);
    }
}

document.addEventListener('DOMContentLoaded', () => {
    console.log('DOM fully loaded');
    addClickListener('generateKeysButton', 'Generate Keys clicked');
    addClickListener('encryptAndPrepareButton', 'Encrypt and Prepare clicked');
    addClickListener('DecryptButton', 'Decrypt clicked');
    document.getElementById('securityLevel').addEventListener('change', setSecurityLevel);
    document.getElementById('generateKeysButton').addEventListener('click', generateKeys);
    document.getElementById('encryptAndPrepareButton').addEventListener('click', encryptAndPrepare);
    document.getElementById('DecryptButton').addEventListener('click', localDecrypt);
});

function log(message) {
    console.log(message);
    ipcRenderer.send('log', message);
}

function timeStart(label) {
    console.time(label);
    ipcRenderer.send('time', label);
}

function timeEnd(label) {
    console.timeEnd(label);
    ipcRenderer.send('timeEnd', label);
}

function setSecurityLevel() {
    const securityLevel = document.getElementById('securityLevel').value;
    fs.writeFile('Data/security_level.txt', securityLevel, (err) => {
        if (err) {
            console.error('Failed to write security level:', err);
            document.getElementById('status').textContent = `Error: Failed to set security level`;
        } else {
            document.getElementById('status').textContent = `Security level is set to ${securityLevel === '0' ? 'TOY' : '128-bit'}`;
        }
    });
}

function encryptFile(filePath, password) {
    try {
        const algorithm = 'aes-256-cbc';
        const key = crypto.scryptSync(password, 'salt', 32);
        const iv = crypto.randomBytes(16);

        const cipher = crypto.createCipheriv(algorithm, key, iv);
        const input = fs.readFileSync(filePath);
        
        const encrypted = Buffer.concat([iv, cipher.update(input), cipher.final()]);
        fs.writeFileSync(filePath + '.enc', encrypted);
        fs.unlinkSync(filePath);
        log(`File encrypted successfully: ${filePath}`);
    } catch (error) {
        log(`Error encrypting file: ${error.message}`);
        throw error;
    }
}

function decryptFile(filePath, password) {
    try {
        const algorithm = 'aes-256-cbc';
        const key = crypto.scryptSync(password, 'salt', 32);

        const encryptedData = fs.readFileSync(filePath);
        const iv = encryptedData.slice(0, 16);
        const encryptedContent = encryptedData.slice(16);

        const decipher = crypto.createDecipheriv(algorithm, key, iv);
        const decrypted = Buffer.concat([decipher.update(encryptedContent), decipher.final()]);
        
        const decryptedFilePath = filePath.slice(0, -4);
        fs.writeFileSync(decryptedFilePath, decrypted);
        log(`File decrypted successfully: ${decryptedFilePath}`);
    } catch (error) {
        log(`Error decrypting file: ${error.message}`);
        throw error;
    }
}

async function generateKeys() {
    const securityLevel = document.getElementById('securityLevel').value;
    const statusElement = document.getElementById('status');

    statusElement.textContent = 'Generating keys...';
    try {
        await localGenerateKeys(securityLevel);
        
        let password = await promptPassword('Enter a password to encrypt your secert key.:');
        
        if (password) {
            encryptFile('Keys/key_sec.txt', password);
            statusElement.textContent = 'Keys generated and key_sec.txt encrypted successfully.';
        } else {
            statusElement.textContent = 'Keys generated, but key_sec.txt was not encrypted (no password provided).';
        }
    } catch (error) {
        console.error('Key generation error:', error);
        statusElement.textContent = `Error: ${error.message}`;
    }
}

async function localDecrypt() {
    const statusElement = document.getElementById('status');
    statusElement.textContent = 'Verifying password...';
    
    try {
        const password = await promptPassword('Enter the password to decrypt key_sec.txt:');

        if (password) {
            try {
                decryptFile('Keys/key_sec.txt.enc', password);
                statusElement.textContent = 'Decrypting...';
                
                const decryptedData = await new Promise((resolve, reject) => {
                    log('Starting decryption...');
                    timeStart('Decryption');
                    exec(`./Decrypt result`, (error, stdout, stderr) => {
                        timeEnd('Decryption');
                        if (error) {
                            log('Decryption failed: ' + stderr);
                            reject(new Error(`Decryption failed: ${stderr}`));
                        } else {
                            fs.readFile('Data/decrypted_result.txt', 'utf8', (err, data) => {
                                if (err) {
                                    log('Failed to read decrypted result: ' + err.message);
                                    reject(err);
                                } else {
                                    log('Decryption completed successfully');
                                    resolve(data);
                                }
                            });
                        }
                    });
                });

                statusElement.textContent = 'Decryption completed successfully.';
                document.getElementById('decryptedResult').textContent = decryptedData;
            } catch (decryptError) {
                throw new Error(`Failed to decrypt key_sec.txt: ${decryptError.message}`);
            }
        } else {
            throw new Error('Decryption cancelled');
        }
    } catch (error) {
        console.error('Decryption error:', error);
        statusElement.textContent = `Error: ${error.message}`;
    } finally {
        if (fs.existsSync('Keys/key_sec.txt')) {
            try {
                const reEncryptPassword = await promptPassword('Enter a password to re-encrypt key_sec.txt:');
                if (reEncryptPassword) {
                    encryptFile('Keys/key_sec.txt', reEncryptPassword);
                    log('key_sec.txt re-encrypted successfully');
                } else {
                    log('Warning: key_sec.txt not re-encrypted');
                }
            } catch (reEncryptError) {
                log(`Error during re-encryption: ${reEncryptError.message}`);
            }
        }
    }
}

function encryptAndPrepare() {
    const query = document.getElementById('query').value.trim();
    const statusElement = document.getElementById('status');

    if (!query) {
        statusElement.textContent = 'Please enter a query.';
        return;
    }

    statusElement.textContent = 'Processing...';
    Promise.all([
        localEncryptQuery(query),
        prepareKeysForUpload()
    ])
        .then(() => {
            statusElement.textContent = 'Query encrypted and keys prepared successfully. You can now upload the files.';
        })
        .catch((error) => {
            console.error('Encryption and preparation error:', error);
            statusElement.textContent = `Error: ${error.message}`;
        });
}

function promptPassword(message) {
    return new Promise((resolve) => {
        const passwordPrompt = document.createElement('div');
        passwordPrompt.innerHTML = `
            <div style="position: fixed; top: 0; left: 0; right: 0; bottom: 0; background: rgba(0,0,0,0.5); display: flex; align-items: center; justify-content: center;">
                <div style="background: white; padding: 20px; border-radius: 5px;">
                    <p>${message}</p>
                    <input type="password" id="passwordInput" />
                    <button id="submitPassword">Submit</button>
                    <button id="cancelPassword">Cancel</button>
                </div>
            </div>
        `;
        document.body.appendChild(passwordPrompt);

        document.getElementById('submitPassword').addEventListener('click', () => {
            const password = document.getElementById('passwordInput').value;
            document.body.removeChild(passwordPrompt);
            resolve(password);
        });

        document.getElementById('cancelPassword').addEventListener('click', () => {
            document.body.removeChild(passwordPrompt);
            resolve(null);
        });
    });
}

function localGenerateKeys(securityLevel) {
    return new Promise((resolve, reject) => {
        log('Starting key generation...');
        timeStart('Key Generation');
        exec(`./Keygen ${securityLevel}`, (error, stdout, stderr) => {
            timeEnd('Key Generation');
            if (error) {
                log('Key generation failed: ' + stderr);
                reject(new Error(`Key generation failed: ${stderr}`));
            } else {
                log('Keys generated successfully');
                resolve();
            }
        });
    });
}

function localEncryptQuery(query) {
    return new Promise((resolve, reject) => {
        log('Starting query encryption...');
        timeStart('Query Encryption');
        fs.writeFile('Data/query.txt', query, (err) => {
            if (err) {
                timeEnd('Query Encryption');
                log('Failed to write query: ' + err.message);
                reject(new Error(`Failed to write query: ${err.message}`));
            } else {
                exec('./Encrypt', (error, stdout, stderr) => {
                    timeEnd('Query Encryption');
                    if (error) {
                        log('Query encryption failed: ' + stderr);
                        reject(new Error(`Query encryption failed: ${stderr}`));
                    } else {
                        log('Query encrypted successfully');
                        resolve();
                    }
                });
            }
        });
    });
}

function prepareKeysForUpload() {
    return new Promise((resolve, reject) => {
        exec('rm -rf Keys_upload required.zip', (rmError, rmStdout, rmStderr) => {
            if (rmError) {
                console.warn(`Warning when removing existing files: ${rmStderr}`);
            }
            
            exec('cp -Rf Keys Keys_upload && rm -f Keys_upload/key_sec.txt', (error, stdout, stderr) => {
                if (error) {
                    reject(new Error(`Failed to prepare keys for upload: ${stderr}`));
                } else {
                    fs.readdir('.', (err, files) => {
                        if (err) {
                            reject(new Error(`Failed to read root directory: ${err.message}`));
                        } else {
                            const cipherFiles = files.filter(file => file.startsWith('cipher'));
                            
                            let zipCommand = 'zip -r required.zip Keys_upload Data/modified_query.txt';
                            cipherFiles.forEach(file => {
                                zipCommand += ` ${file}`;
                            });
                            
                            exec(zipCommand, (zipError, zipStdout, zipStderr) => {
                                if (zipError) {
                                    reject(new Error(`Failed to create zip file: ${zipStderr}`));
                                } else {
                                    log('Zip file created successfully');
                                    timeEnd('Query Encryption');
                                    resolve();
                                }
                            });
                        }
                    });
                }
            });
        });
    });
}