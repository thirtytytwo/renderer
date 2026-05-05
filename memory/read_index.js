const fs = require('fs');
const path = require('path');

const buf = fs.readFileSync(path.join(__dirname, 'codeindex.bin'));
let offset = 0;

function readUint8() { const v = buf.readUInt8(offset); offset += 1; return v; }
function readUint16() { const v = buf.readUInt16LE(offset); offset += 2; return v; }
function readUint32() { const v = buf.readUInt32LE(offset); offset += 4; return v; }
function readString() {
  const len = readUint16();
  const s = buf.toString('utf-8', offset, offset + len);
  offset += len;
  return s;
}
function readLineRange() { const s = readUint32(); const e = readUint32(); return `[${s}-${e}]`; }

// header
const magic = buf.toString('ascii', 0, 4); offset += 4;
const version = readUint16();
const sectionCount = readUint32();

console.log(`Magic: ${magic}, Version: ${version}, Sections: ${sectionCount}\n`);

const SECTION_NAMES = { 1:'META', 2:'FILE_TREE', 3:'MODULE', 4:'DEP_GRAPH', 5:'DATA_FLOW', 6:'STATUS', 7:'CONVENTION' };
const STATUS_NAMES = { 1:'OK', 2:'WARN', 3:'ERR' };

for (let i = 0; i < sectionCount; i++) {
  const type = readUint8();
  const dataLen = readUint32();
  const sectionEnd = offset + dataLen;
  const name = SECTION_NAMES[type] || `UNKNOWN(0x${type.toString(16)})`;
  console.log(`=== ${name} (${dataLen} bytes) ===`);

  if (type === 0x01) { // META
    const count = readUint16();
    for (let j = 0; j < count; j++) {
      const k = readString(), v = readString();
      console.log(`  ${k}: ${v}`);
    }
  } else if (type === 0x02) { // FILE_TREE
    const count = readUint16();
    for (let j = 0; j < count; j++) {
      const p = readString(), d = readString(), lr = readLineRange(), k = readString();
      console.log(`  ${p} ${lr} [${k}] - ${d}`);
    }
  } else if (type === 0x03) { // MODULE
    const count = readUint16();
    for (let j = 0; j < count; j++) {
      const n = readString(), f = readString(), desc = readString(), cc = readString(), notes = readString();
      console.log(`  Module: ${n} (${f})`);
      console.log(`    Desc: ${desc}`);
      if (cc) console.log(`    Call: ${cc}`);
      if (notes) console.log(`    Notes: ${notes}`);
      const rc = readUint16();
      for (let r = 0; r < rc; r++) { const lr = readLineRange(); const d = readString(); console.log(`    ${lr}: ${d}`); }
      const sc = readUint16();
      for (let s = 0; s < sc; s++) {
        const sn = readString(), slr = readLineRange(), sd = readString();
        console.log(`    Sub: ${sn} ${slr} - ${sd}`);
        const src = readUint16();
        for (let r = 0; r < src; r++) { const lr = readLineRange(); const d = readString(); console.log(`      ${lr}: ${d}`); }
      }
    }
  } else if (type === 0x04) { // DEP_GRAPH
    const count = readUint16();
    for (let j = 0; j < count; j++) {
      const from = readString(), to = readString(), t = readString();
      console.log(`  ${from} -> ${to} [${t}]`);
    }
  } else if (type === 0x05) { // DATA_FLOW
    const count = readUint16();
    for (let j = 0; j < count; j++) {
      const step = readUint8(), desc = readString();
      console.log(`  Step ${step}: ${desc}`);
    }
  } else if (type === 0x06) { // STATUS
    const count = readUint16();
    for (let j = 0; j < count; j++) {
      const n = readString(), s = readUint8(), d = readString();
      console.log(`  ${n}: [${STATUS_NAMES[s]||s}] ${d}`);
    }
  } else if (type === 0x07) { // CONVENTION
    const count = readUint16();
    for (let j = 0; j < count; j++) {
      const k = readString(), v = readString();
      console.log(`  ${k}: ${v}`);
    }
  }

  offset = sectionEnd;
  console.log('');
}

console.log(`Total bytes read: ${offset} / ${buf.length}`);