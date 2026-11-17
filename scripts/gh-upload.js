// Upload a local git working tree to GitHub via the Git Data API.
// Usage: node gh-upload.js <repoDir> <owner> <repo> <branch> <message>
// Requires GH_TOKEN in the environment.
const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');

const [repoDir, owner, repo, branch, message] = process.argv.slice(2);
const token = process.env.GH_TOKEN;
if (!token) { console.error('GH_TOKEN missing'); process.exit(1); }

const API = 'https://api.github.com';
const headers = {
  Authorization: `token ${token}`,
  'User-Agent': 'dsh-gh-upload',
  Accept: 'application/vnd.github+json',
  'Content-Type': 'application/json',
};

async function call(method, url, body) {
  let lastErr = null;
  for (let attempt = 0; attempt < 8; attempt++) {
    let res;
    try {
      res = await fetch(API + url, { method, headers, body: body ? JSON.stringify(body) : undefined });
    } catch (err) {
      lastErr = err;
      await new Promise(r => setTimeout(r, 1000 * (attempt + 1)));
      continue;
    }
    if (res.status === 403 || res.status === 429 || res.status >= 500) {
      await new Promise(r => setTimeout(r, 1000 * (attempt + 1)));
      continue;
    }
    const text = await res.text();
    if (!res.ok) throw new Error(`${method} ${url} -> ${res.status} ${text.slice(0, 300)}`);
    return text ? JSON.parse(text) : null;
  }
  throw new Error(`${method} ${url} -> retries exhausted: ${(lastErr && (lastErr.cause ? lastErr.cause.code || lastErr.cause : lastErr.message)) || 'unknown'}`);
}

const raw = execSync('git ls-files -z', { cwd: repoDir, maxBuffer: 64 * 1024 * 1024 });
const files = raw.toString('utf8').split('\0').filter(Boolean);
console.log('files: ' + files.length);

const blobs = new Array(files.length);
let cursor = 0;
let done = 0;

async function worker() {
  while (true) {
    const i = cursor++;
    if (i >= files.length) return;
    const rel = files[i];
    const abs = path.join(repoDir, rel);
    const content = fs.readFileSync(abs);
    const out = await call('POST', `/repos/${owner}/${repo}/git/blobs`, {
      content: content.toString('base64'),
      encoding: 'base64',
    });
    blobs[i] = { path: rel, mode: '100644', type: 'blob', sha: out.sha };
    done++;
    if (done % 100 === 0) console.log(`blobs ${done}/${files.length}`);
  }
}

(async () => {
  await Promise.all(Array.from({ length: 6 }, worker));
  console.log('blobs done, creating tree');
  const tree = await call('POST', `/repos/${owner}/${repo}/git/trees`, { tree: blobs });
  console.log('tree ' + tree.sha);
  let parents = [];
  try {
    const head = await call('GET', `/repos/${owner}/${repo}/git/ref/heads/${branch}`);
    parents = [head.object.sha];
    console.log('parent ' + head.object.sha);
  } catch (e) {
    console.log('no parent ref: ' + e.message.slice(0, 120));
  }
  const commit = await call('POST', `/repos/${owner}/${repo}/git/commits`, {
    message,
    tree: tree.sha,
    parents,
  });
  console.log('commit ' + commit.sha);
  if (parents.length) {
    await call('PATCH', `/repos/${owner}/${repo}/git/refs/heads/${branch}`, { sha: commit.sha, force: true });
  } else {
    await call('POST', `/repos/${owner}/${repo}/git/refs`, { ref: `refs/heads/${branch}`, sha: commit.sha });
  }
  console.log('ref refs/heads/' + branch + ' -> ' + commit.sha);
})().catch(e => { console.error('FAILED: ' + e.message); process.exit(1); });