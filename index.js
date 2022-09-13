const SegfaultHandler = require('segfault-handler');
SegfaultHandler.registerHandler('crash.log');

var KEngineCore = require("bindings")("KEngineCoreNode");

const fs = require('fs').promises;
const path = require('path');
const process = require('process');
const { authenticate } = require('@google-cloud/local-auth');
const { google } = require('googleapis');

// If modifying these scopes, delete token.json.
const SCOPES = ['https://www.googleapis.com/auth/spreadsheets.readonly'];
// The file token.json stores the user's access and refresh tokens, and is
// created automatically when the authorization flow completes for the first
// time.
const TOKEN_PATH = path.join(process.cwd(), 'token.json');
const CREDENTIALS_PATH = path.join(process.cwd(), 'credentials.json');

const spreadsheetId = process.argv[2];
const outputFilename = process.argv[3];
if (!spreadsheetId || !outputFilename) {
    console.log("Usage : node . [spreadsheetId] [outputFile]");
    process.exit(9);
}



/**
 * Reads previously authorized credentials from the save file.
 *
 * @return {Promise<OAuth2Client|null>}
 */
async function loadSavedCredentialsIfExist() {
    try {
        const content = await fs.readFile(TOKEN_PATH);
        const credentials = JSON.parse(content);
        return google.auth.fromJSON(credentials);
    } catch (err) {
        return null;
    }
}

/**
 * Serializes credentials to a file comptible with GoogleAUth.fromJSON.
 *
 * @param {OAuth2Client} client
 * @return {Promise<void>}
 */
async function saveCredentials(client) {
    const content = await fs.readFile(CREDENTIALS_PATH);
    const keys = JSON.parse(content);
    const key = keys.installed || keys.web;
    const payload = JSON.stringify({
        type: 'authorized_user',
        client_id: key.client_id,
        client_secret: key.client_secret,
        refresh_token: client.credentials.refresh_token,
    });
    await fs.writeFile(TOKEN_PATH, payload);
}

/**
 * Load or request or authorization to call APIs.
 *
 */
async function authorize() {
    let client = await loadSavedCredentialsIfExist();
    if (client) {
        return client;
    }
    client = await authenticate({
        scopes: SCOPES,
        keyfilePath: CREDENTIALS_PATH,
    });
    if (client.credentials) {
        await saveCredentials(client);
    }
    return client;
}

/**
 * Gets all sheets from the spreadsheet and parses them to create a DataTree.
 * @param {google.auth.OAuth2} auth The authenticated Google OAuth client.
 */
async function growTree(auth) {
    const sheets = google.sheets({ version: 'v4', auth });

    const request = {
        // The spreadsheet to request.
        spreadsheetId: spreadsheetId, 
        includeGridData: true,  
        auth: auth
    };

    try {
        const response = (await sheets.spreadsheets.get(request)).data;
        let root = new KEngineCore.DataSapling();
        root.addKey("sheetName");
        var sheetsMeta = {};
        response.sheets.forEach((sheet) => {
            let mainBranch = root.growBranch();
            var sheetName = sheet.properties.title;
            mainBranch.setHash("sheetName", sheetName);
            root.branchReady(mainBranch);

            var branchHeader = mainBranch.createBranchHeader();

            var sheetMeta = [];
            sheetsMeta[sheetName] = sheetMeta;
            var typeHeaderRow = sheet.data[0].rowData[0].values.map(a => a.effectiveValue.stringValue);
            var nameHeaderRow = sheet.data[0].rowData[1].values.map(a => a.effectiveValue.stringValue);

            for (var index = 0; index < typeHeaderRow.length && index < nameHeaderRow.length; index++) {
                var ref = undefined;
                if (typeHeaderRow[index].startsWith("Ref")) {
                    var firstIndex = 4;
                    var lastIndex = typeHeaderRow[index].length - 1;
                    ref = typeHeaderRow[index].slice(firstIndex, lastIndex);
                }
                const type = ref ? "Hash" : typeHeaderRow[index];
                const name = nameHeaderRow[index];
                sheetMeta.push({ type: type, name: name, ref: ref, refList: ref ? [] : undefined });
                switch (type) {
                    case "Key":
                        branchHeader.addHash(name);
                        mainBranch.addKey(name);
                        break;
                    case "Hash":
                        branchHeader.addHash(name);
                        break;
                    case "Int":
                        branchHeader.addInt(name);
                        break;
                    case "Float":
                        branchHeader.addFloat(name);
                        break;
                    case "Bool":
                        branchHeader.addBool(name);
                        break;
                    case "String":
                        branchHeader.addString(name);
                        break;
                    default:
                        console.warn("Unrecognized header type: " + type);
                        break;
                }
            }
            var rows = sheet.data[0].rowData.slice(2);

            rows.forEach((row) => {
                var values = row.values.map(a => a.effectiveValue);
                var branch = mainBranch.growBranch();
                for (var index = 0; index < sheetMeta.length; index++) {
                    const type = sheetMeta[index].type;
                    const name = sheetMeta[index].name;
                    const valueChunk = values[index];
                    switch (sheetMeta[index].type) {
                        case "Key":
                        case "Hash":
                            branch.setHash(name, valueChunk.stringValue);
                            if (sheetMeta[index].ref) {
                                sheetMeta[index].refList.push(valueChunk.stringValue);
                            }
                            break;
                        case "Int":
                            if (!Number.isInteger(valueChunk.numberValue)) {
                                console.warn("Integer cell has unexpected value " + valueChunk.numberValue);
                            }
                            branch.setInt(name, valueChunk.numberValue);
                            break;
                        case "Float":
                            branch.setFloat(name, valueChunk.numberValue);
                            break;
                        case "Bool":
                            var isFalse = valueChunk.stringValue == "false" || valueChunk.stringValue == "FALSE";
                            var isTruthy = valueChunk.stringValue || valueChunk.numberValue;
                            if (valueChunk.stringValue && valueChunk.stringValue != "true" && valueChunk.stringValue != "TRUE" && valueChunk.stringValue != "false" && valueChunk.stringValue != "FALSE") {
                                console.warn("Boolean cell has unexpected string " + valueChunk.stringValue + ", treated as 'true'");
                            }
                            branch.setBool(name, !isFalse && isTruthy);
                            break;
                        case "String":
                            branch.setString(name, valueChunk.stringValue);
                            break;
                    }
                }
                mainBranch.branchReady(branch);
            });

            Object.keys(sheetsMeta).forEach((sheetName) => {
                var sheetMeta = sheetsMeta[sheetName];
                sheetMeta.forEach((col) => {
                    if (col.ref) {
                        var refBranchName = col.ref.split(",")[0];
                        var refKey = col.ref.split(",")[1];
                        if (!root.hasBranch("sheetName", refBranchName)) {
                            console.warn("missing Sheet ref: " + refBranchName);
                        } else {
                            var referencedBranch = root.getBranch("sheetName", refBranchName);
                            col.refList.forEach((refString) => {
                                if (!referencedBranch.hasBranch(refKey, refString)) {
                                    console.warn("missing ref.  Sheet: " + refBranchName + " Key: " + refKey + " Value: " + refString);
                                }
                            });
                        }
                    }
                });
            });

        });

        root.writeToFile(outputFilename);

    } catch (err) {
        console.error(err);
    }
}

authorize().then(growTree).catch(console.error);
