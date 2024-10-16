const express = require('express');
const path = require('path');
const { exec } = require('child_process');
const fs = require('fs');
const multer = require('multer');
const https = require('https');
const http = require('http');
const bcrypt = require('bcrypt');
const app = express();
app.use(express.json());

// Set up multer for CSV file uploads

const options = {
  key: fs.readFileSync(path.join(__dirname, 'private.key')),
  cert: fs.readFileSync(path.join(__dirname, 'certificate.crt')),
  //ca: fs.readFileSync(path.join(__dirname, 'ca_bundle.crt')) // Optional, if using intermediate CA
};

const allowedIP = '140.122.185.199';
const httpsPort = process.env.HTTPS_PORT || 8080;
const httpPort = process.env.HTTP_PORT || 3000;

// IP restriction middleware
app.use((req, res, next) => {
  const clientIP = req.ip;
  if (clientIP === allowedIP) {
    next();
  } else {
    next();
    //res.status(403).send('Access forbidden: IP address not allowed');
  }
});

// Store user data with hashed passwords
const users = {
  'user1': { hash: bcrypt.hashSync('password1', 10), failedAttempts: 0, blockedUntil: 0 },
  'user2': { hash: bcrypt.hashSync('password2', 10), failedAttempts: 0, blockedUntil: 0 }
};

const MAX_FAILED_ATTEMPTS = 5;
const BLOCK_DURATION = 15 * 60 * 1000; // 15 minutes in milliseconds

// Handle login request
app.post('/login', (req, res) => {
  const { username, password } = req.body;
  const user = users[username];

  if (!user) {
    return res.json({ success: false, message: 'Invalid username or password' });
  }

  const now = Date.now();

  // Check if the user is blocked
  if (now < user.blockedUntil) {
    const remainingTime = Math.ceil((user.blockedUntil - now) / 1000 / 60);
    return res.json({ success: false, message: `Account is blocked. Try again in ${remainingTime} minutes.` });
  }

  bcrypt.compare(password, user.hash, (err, result) => {
    if (result) {
      // Successful login
      user.failedAttempts = 0;
      res.json({ success: true });
    } else {
      // Failed login
      user.failedAttempts++;

      if (user.failedAttempts >= MAX_FAILED_ATTEMPTS) {
        user.blockedUntil = now + BLOCK_DURATION;
        user.failedAttempts = 0;
        res.json({ success: false, message: 'Too many failed attempts. Account is blocked for 15 minutes.' });
      } else {
        res.json({ success: false, message: 'Invalid username or password' });
      }
    }
  });
});

const storage = multer.diskStorage({
  destination: function (req, file, cb) {
    cb(null, './')
  },
  filename: function (req, file, cb) {
    cb(null, file.originalname)
  }
});

const upload = multer({
  storage: storage,
  fileFilter: function (req, file, cb) {
    if (path.extname(file.originalname).toLowerCase() !== '.csv') {
      return cb(new Error('Only CSV files are allowed'))
    }
    cb(null, true)
  }
});

const zipUpload = multer({
  storage: storage,
  fileFilter: function (req, file, cb) {
    if (path.extname(file.originalname).toLowerCase() !== '.zip') {
      return cb(new Error('Only ZIP files are allowed'))
    }
    cb(null, true)
  }
});

// Serve static files from the public directory
app.use(express.static(path.join(__dirname, 'public')));

// Parse JSON request bodies
app.use(express.json());

console.log('Server starting...');

app.post('/upload-zip', zipUpload.single('file'), (req, res) => {
  if (!req.file) {
    return res.status(400).json({ error: 'No file uploaded or file is not a zip' });
  }
  res.json({ message: 'Zip file uploaded successfully', filename: req.file.filename });
});

// Route to execute the 'Search' program
app.post('/execute', (req, res) => {
  const { zipFilename, csvFilename } = req.body;

  // if (!zipFilename) {
  //   return res.status(400).json({ error: 'No zip file specified' });
  // }

  // if (!csvFilename) {
  //   return res.status(400).json({ error: 'No CSV file specified' });
  // }

  const zipPath = path.join(__dirname, zipFilename);
  const csvPath = path.join(__dirname, csvFilename);

  if (!fs.existsSync(zipPath)) {
    return res.status(404).json({ error: 'Zip file not found' });
  }

  if (!fs.existsSync(csvPath)) {
    return res.status(404).json({ error: 'CSV file not found' });
  }

  // Unzip the file
  exec(`unzip -o "${zipPath}" -d ./`, (unzipError, unzipStdout, unzipStderr) => {
    if (unzipError) {
      console.error('Unzip error:', unzipError);
      return res.status(500).json({ error: 'Failed to unzip file' });
    }

    // Execute the Search program with the CSV file as a parameter
    exec(`./Search ${csvFilename}`, (searchError, searchStdout, searchStderr) => {
      if (searchError) {
        console.error('Search error:', searchError);
        return res.status(500).json({ error: 'Failed to execute search' });
      }

      res.json({ message: 'Search completed successfully' });
    });
  });
});

// New route to download the result
app.get('/download-result', (req, res) => {
  const resultPath = path.join(__dirname, 'result');

  if (fs.existsSync(resultPath)) {
    res.download(resultPath, 'result', (err) => {
      if (err) {
        console.error('Download error:', err);
        res.status(500).json({ error: 'Failed to download result' });
      }
    });
  } else {
    res.status(404).json({ error: 'Result file not found' });
  }
});


app.get('/file-content/:filename', (req, res) => {
  const filename = req.params.filename;
  const filePath = path.join(__dirname, './', filename);

  console.log(`Requested file content: ${filename}`);

  fs.readFile(filePath, 'utf8', (err, data) => {
    if (err) {
      console.error(`Error reading file ${filename}:`, err);
      res.status(500).json({ error: 'Error reading file' });
    } else {
      console.log(`File ${filename} read successfully`);
      res.json({ content: data });
    }
  });
});


// Route to handle CSV file uploads
app.post('/upload', upload.single('file'), (req, res) => {
  if (!req.file) {
    return res.status(400).json({ error: 'No file uploaded or file is not a CSV' });
  }
  res.json({ message: 'CSV file uploaded successfully', filename: req.file.filename });
});

// Route to list CSV files in the uploads directory
app.get('/files', (req, res) => {
  console.log('Listing files in Data directory');
  fs.readdir('./', (err, files) => {
    if (err) {
      console.error('Error reading Data directory:', err);
      return res.status(500).json({ error: 'Error reading directory' });
    }
    const csvFiles = files.filter(file => path.extname(file).toLowerCase() === '.csv');
    console.log('CSV files found:', csvFiles);
    res.json({ files: csvFiles });
  });
});

https.createServer(options, app).listen(httpsPort, allowedIP, () => {
  console.log(`HTTPS Server running on https://${allowedIP}:${httpsPort}`);
});


// Redirect HTTP to HTTPS
http.createServer((req, res) => {
  console.log(`Redirecting HTTP request to HTTPS: ${req.url}`);
  res.writeHead(301, { "Location": `https://${allowedIP}:${httpsPort}${req.url}` });
  res.end();
}).listen(httpPort, allowedIP, () => {
  console.log(`HTTP server listening on ${allowedIP}:${httpPort} and redirecting to https://${allowedIP}:${httpsPort}`);
});



