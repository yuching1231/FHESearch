const { app, BrowserWindow, ipcMain, dialog } = require('electron');
const path = require('path');

function createWindow () {
    const win = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            nodeIntegration: true,
            contextIsolation: false,
            enableRemoteModule: true
        }
    });

    win.loadFile('local.html');
    //win.webContents.openDevTools(); // This will open the DevTools by default

    win.webContents.on('dom-ready', () => {
        win.webContents.executeJavaScript(`
            console.log('DOM content:', document.body.innerHTML);
        `);
    });

    win.webContents.on('did-finish-load', () => {
        console.log('Window loaded successfully');
    });

    win.webContents.on('did-fail-load', (event, errorCode, errorDescription) => {
        console.error('Failed to load window:', errorDescription);
    });
}

app.whenReady().then(() => {
    createWindow();

    app.on('activate', () => {
        if (BrowserWindow.getAllWindows().length === 0) {
            createWindow();
        }
    });
});

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') {
        app.quit();
    }
});

ipcMain.on('log', (event, message) => {
    console.log(message);
});

ipcMain.handle('show-dialog', (event, options) => {
    return dialog.showMessageBox(options);
});