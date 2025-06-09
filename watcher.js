const { MongoClient } = require('mongodb');
const axios = require('axios');
const fs = require('fs');
const path = require('path');

const uri = 'mongodb+srv://barana340:barana340@tryon.7vvdbo9.mongodb.net/?retryWrites=true&w=majority&appName=tryon';
const dbName = 'tryonDB';
const collectionName = 'imagescans';

// Downloads klasörünü oluşturalım
const DOWNLOAD_DIR = path.join(__dirname, 'downloads');
const METADATA_PATH = path.join(__dirname, 'last_download.json');

// downloads klasörünü oluştur
if (!fs.existsSync(DOWNLOAD_DIR)) {
    fs.mkdirSync(DOWNLOAD_DIR, { recursive: true });
}

// Metadata kaydetme fonksiyonu
async function saveMetadata(category, filename) {
    const metadata = {
        category: category,
        filename: filename,
        fullPath: path.join(DOWNLOAD_DIR, filename),
        timestamp: new Date().toISOString()
    };
    await fs.promises.writeFile(
        METADATA_PATH,
        JSON.stringify(metadata, null, 2)
    );
    console.log('Metadata saved:', metadata);
}

async function downloadFile(url, filename = 'downloaded_file') {
    const filePath = path.join(DOWNLOAD_DIR, filename);
    const response = await axios({
        method: 'GET',
        url: url,
        responseType: 'stream',
    });

    const writer = fs.createWriteStream(filePath);

    response.data.pipe(writer);

    return new Promise((resolve, reject) => {
        writer.on('finish', () => {
            console.log(`Downloaded: ${url} → ${filePath}`);
            resolve(filePath);
        });
        writer.on('error', reject);
    });
}

async function main() {
    const client = new MongoClient(uri);
    await client.connect();
    console.log('Connected to MongoDB');

    const collection = client.db(dbName).collection(collectionName);
    const changeStream = collection.watch([{ $match: { operationType: 'insert' } }]);

    console.log('Watching for new documents in collection:', collectionName);

    changeStream.on('change', async (change) => {
        console.log('New document inserted:', change.fullDocument);
        const fileUrl = change.fullDocument.imageUrl;
        const category = change.fullDocument.category;

        if (fileUrl) {
            try {
                const filename = `downloaded_${category}_${Date.now()}.jpg`; // veya uygun uzantıyı ekleyin
                await downloadFile(fileUrl, filename);
                await saveMetadata(category, filename);
            } catch (err) {
                console.error('Error:', err.message);
            }
        }
    });

    // Hata yönetimi
    changeStream.on('error', (error) => {
        console.error('Change stream error:', error);
    });
}

main().catch(console.error);
