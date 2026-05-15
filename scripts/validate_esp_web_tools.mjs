import assert from 'node:assert/strict'
import { readFile } from 'node:fs/promises'

const root = new URL('..', import.meta.url)

async function readProjectFile(path) {
    return readFile(new URL(path, root), 'utf8')
}

async function readJsonFile(path) {
    return JSON.parse(await readProjectFile(path))
}

const packageJson = await readJsonFile('package.json')
const manifest = await readJsonFile('web/esp-web-tools/manifest.json')
const html = await readProjectFile('web/esp-web-tools/index.html')
const htmlText = html.replace(/\s+/g, ' ')
const css = await readProjectFile('web/esp-web-tools/style.css')
const icon = await readProjectFile('web/esp-web-tools/neon-meter-icon.svg')
const workflow = await readProjectFile('.github/workflows/deploy-web-tools.yml')

assert.equal(manifest.name, 'Neon Meter')
assert.equal(manifest.version, packageJson.version)
assert.equal(manifest.new_install_prompt_erase, true)
assert.equal(manifest.new_install_improv_wait_time, 0)

assert.equal(manifest.builds.length, 1)
assert.equal(manifest.builds[0].chipFamily, 'ESP32-S3')
assert.deepEqual(manifest.builds[0].parts, [
    {
        path: 'firmware/neon-meter-m5stack-cores3.factory.bin',
        offset: 0,
    },
])

assert.match(html, /esp-web-tools@10\/dist\/web\/install-button\.js\?module/)
assert.match(html, /<esp-web-install-button\s+manifest="manifest\.json"/)
assert.match(html, /href="neon-meter-icon\.svg"/)
assert.match(html, /class="brand-icon"/)
assert.match(htmlText, /especially designed for the M5Stack CoreS3/)
assert.match(htmlText, /Connect your M5Stack CoreS3/)
assert.match(css, /--accent-cyan: #00f5ff;/)
assert.match(css, /\.brand-icon/)
assert.match(css, /\.btn-primary/)
assert.match(icon, /<title>Neon Meter<\/title>/)
assert.match(icon, /Dark neon gauge icon/)

assert.match(workflow, /branches: \[main\]/)
assert.match(workflow, /npm run build/)
assert.match(workflow, /firmware\.factory\.bin/)
assert.match(workflow, /actions\/deploy-pages@v4/)
assert.match(workflow, /pages: write/)

console.log(
    `Validated ESP Web Tools assets for ${manifest.name} ${manifest.version}`,
)
