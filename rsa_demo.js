// rsa_demo.js
// Использование: node rsa_demo.js <n> <e> <d> "Сообщение"

function modPow(base, exp, mod) {
  // Быстрое возведение в степень по модулю
  let result = 1n;
  let b = base % mod;
  let e = exp;
  while (e > 0n) {
    if (e & 1n) {
      result = (result * b) % mod;
    }
    b = (b * b) % mod;
    e >>= 1n;
  }
  return result;
}

// Преобразуем строку в BigInt (наивный способ через char-коды)
function stringToBigInt(str) {
  let res = 0n;
  for (let i = 0; i < str.length; i++) {
    res = (res << 8n) + BigInt(str.charCodeAt(i));
  }
  return res;
}

// Обратное преобразование BigInt -> строка
function bigIntToString(big) {
  let result = "";
  let curr = big;
  while (curr > 0n) {
    let ch = Number(curr & 0xFFn);
    result = String.fromCharCode(ch) + result;
    curr >>= 8n;
  }
  return result;
}

function main() {
  if (process.argv.length < 6) {
    console.log("Usage: node rsa_demo.js <n> <e> <d> <message>");
    process.exit(1);
  }

  const n = BigInt(process.argv[2]);
  const e = BigInt(process.argv[3]);
  const d = BigInt(process.argv[4]);
  const message = process.argv[5];

  console.log("Original message:", message);

  // 1) Преобразуем строку в BigInt
  const m = stringToBigInt(message);
  console.log("Message as BigInt:", m.toString());

  // 2) Шифруем (c = m^e mod n)
  const c = modPow(m, e, n);
  console.log("Cipher (encrypted):", c.toString());

  // 3) Расшифровываем (m2 = c^d mod n)
  const m2 = modPow(c, d, n);
  console.log("Decrypted BigInt:", m2.toString());

  // 4) Преобразуем обратно в строку
  const decryptedMessage = bigIntToString(m2);
  console.log("Decrypted message:", decryptedMessage);
}

main();