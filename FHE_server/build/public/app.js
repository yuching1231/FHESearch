const API_URL = `https://${window.location.hostname}:8080`;

document.addEventListener('DOMContentLoaded', () => {
  document.getElementById('login-form').addEventListener('submit', handleLogin);
  document.getElementById('uploadButton').addEventListener('click', uploadFile);
  document.getElementById('uploadZipButton').addEventListener('click', uploadZipFile);
  document.getElementById('executeButton').addEventListener('click', executeSearch);
  document.getElementById('downloadResultButton').addEventListener('click', downloadResult);
  document.getElementById('fileSelect').addEventListener('change', displayFileContent);
  listFiles();
});

function handleLogin(e) {
  e.preventDefault();
  const username = document.getElementById('username').value;
  const password = document.getElementById('password').value;

  axios.post(`${API_URL}/login`, { username, password })
    .then(response => {
      if (response.data.success) {
        document.getElementById('login-container').style.display = 'none';
        document.getElementById('main-content').style.display = 'block';
      } else {
        alert(response.data.message || 'Invalid username or password');
      }
    })
    .catch(error => {
      console.error('Login error:', error);
      alert(error.response?.data?.message || 'An error occurred during login');
    });
}


document.getElementById('login-form').addEventListener('submit', function(e) {
  e.preventDefault();
  const username = document.getElementById('username').value;
  const password = document.getElementById('password').value;

  axios.post(`${API_URL}/login`, { username, password })
    .then(response => {
      if (response.data.success) {
        document.getElementById('login-container').style.display = 'none';
        document.getElementById('main-content').style.display = 'block';
      } else {
        alert('Invalid username or password');
      }
    })
    .catch(error => {
      console.error('Login error:', error);
      alert('An error occurred during login');
    });
});

async function generateKeys() {
  const securityLevel = document.getElementById('securityLevel').value;

  try {
      document.querySelector('.loading-container').style.display = 'flex';
      const response = await axios.post(`${API_URL}/generate-keys`, {securityLevel});
      alert(response.data.message);
  } catch (error) {
      console.error('Key generation error:', error);
      alert(`Error: ${error.response?.data?.error || error.message}`);
  } finally {
      document.querySelector('.loading-container').style.display = 'none';
  }
}

async function uploadZipFile() {
  const fileInput = document.getElementById('zipFileInput');
  const file = fileInput.files[0];

  if (!file) {
    alert('Please select a zip file to upload');
    return;
  }

  if (file.name.split('.').pop().toLowerCase() !== 'zip') {
    alert('Please select a zip file');
    return;
  }

  const formData = new FormData();
  formData.append('file', file);

  try {
    document.querySelector('.loading-container').style.display = 'flex';

    const response = await axios.post(`${API_URL}/upload-zip`, formData, {
      headers: { 'Content-Type': 'multipart/form-data' }
    });
    alert(response.data.message);
    // Set a flag in localStorage to indicate successful upload
    localStorage.setItem('zipFileUploaded', 'true');
    // Store the filename
    localStorage.setItem('uploadedZipFilename', response.data.filename);
  } catch (error) {
    console.error('Upload error:', error);
    alert(`Error: ${error.response?.data?.error || error.message}`);
  } finally {
    document.querySelector('.loading-container').style.display = 'none';
  }
}

async function executeSearch() {
  const zipFileUploaded = localStorage.getItem('zipFileUploaded') === 'true';
  const zipFilename = localStorage.getItem('uploadedZipFilename');
  const selectedCsvFile = document.getElementById('fileSelect').value;

  if (!zipFileUploaded || !zipFilename) {
    alert('Please upload a zip file first');
    return;
  }

  if (!selectedCsvFile) {
    alert('Please select a CSV file');
    return;
  }

  try {
    document.querySelector('.loading-container').style.display = 'flex';
    const response = await axios.post(`${API_URL}/execute`, { zipFilename, csvFilename: selectedCsvFile });
    alert(response.data.message);
    document.getElementById('downloadResultButton').disabled = false;
  } catch (error) {
    console.error('Execution error:', error);
    alert(`Error: ${error.response?.data?.error || error.message}`);
  } finally {
    document.querySelector('.loading-container').style.display = 'none';
  }
}

function downloadResult() {
  window.location.href = `${API_URL}/download-result`;
}



async function uploadFile() {
  const fileInput = document.getElementById('fileInput');
  const file = fileInput.files[0];

  if (!file) {
    alert('Please select a CSV file to upload');
    return;
  }

  if (file.name.split('.').pop().toLowerCase() !== 'csv') {
    alert('Please select a CSV file');
    return;
  }

  const formData = new FormData();
  formData.append('file', file);

  try {
    const response = await axios.post(`${API_URL}/upload`, formData, {
      headers: { 'Content-Type': 'multipart/form-data' }
    });
    alert(response.data.message);
    listFiles();
  } catch (error) {
    console.error('Upload error:', error);
    alert(`Error: ${error.response?.data?.error || error.message}`);
  }
}

async function listFiles() {
  try {
    const response = await axios.get(`${API_URL}/files`);
    const fileSelect = document.getElementById('fileSelect');
    fileSelect.innerHTML = '<option value="">Select a file</option>';
    response.data.files.forEach(file => {
      const option = document.createElement('option');
      option.value = file;
      option.textContent = file;
      fileSelect.appendChild(option);
    });
  } catch (error) {
    console.error('Error listing files:', error);
    alert(`Error: ${error.response?.data?.error || error.message}`);
  }
}

function clearUploadStatus() {
  localStorage.removeItem('zipFileUploaded');
  localStorage.removeItem('uploadedZipFilename');
}

// Call this when a new file is selected
document.getElementById('zipFileInput').addEventListener('change', clearUploadStatus);

// Also call this when the page loads
document.addEventListener('DOMContentLoaded', clearUploadStatus);

async function displayFileContent() {
  const selectedFile = document.getElementById('fileSelect').value;
  if (!selectedFile) {
    document.getElementById('fileContent').textContent = 'No file selected';
    return;
  }

  try {
    const response = await axios.get(`${API_URL}/file-content/${selectedFile}`);
    document.getElementById('fileContent').textContent = response.data.content;
  } catch (error) {
    console.error('Error fetching file content:', error);
    document.getElementById('fileContent').textContent = `Error: ${error.response?.data?.error || error.message}`;
  }
}


