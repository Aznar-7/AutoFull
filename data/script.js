const connEl = document.getElementById('conn');
const connDot = connEl.querySelector('.dot');
const connText = connEl.querySelector('.text') || connEl;
const stateEl = document.getElementById('state');
const sensorEl = document.getElementById('sensor');
const tMeter = document.getElementById('t-meter')?.querySelector('.meter-fill');
const hMeter = document.getElementById('h-meter')?.querySelector('.meter-fill');
const buttons = document.querySelectorAll('.btn[data-cmd]');
let isConnected = false;
let lastCmd = '';
let cmdCooldown = 80; // ms to avoid flooding
let lastCmdTime = 0;

async function sendCmd(cmd) {
    const now = Date.now();
    if (cmd === lastCmd && (now - lastCmdTime) < cmdCooldown) return;
    lastCmd = cmd;
    lastCmdTime = now;

    try {
        const controller = new AbortController();
        const id = setTimeout(() => controller.abort(), 1200);
        await fetch(`/command?cmd=${encodeURIComponent(cmd)}`, {method: 'GET', cache: 'no-cache', signal: controller.signal});
        clearTimeout(id);
        connText.textContent = 'conectado';
        connEl.setAttribute('aria-status','ok');
        isConnected = true;
    } catch (e) {
        connText.textContent = 'desconectado';
        connEl.removeAttribute('aria-status');
        isConnected = false;
    }
}

function parseStatus(txt) {
    const trimmed = (txt||'').trim();
    if (!trimmed) return {state: '—', sensor: '—', temp: '—', hum: '—'};
    const tMatch = trimmed.match(/T:([0-9]+(?:\.[0-9]+)?)/);
    const hMatch = trimmed.match(/H:([0-9]+(?:\.[0-9]+)?)/);
    const sMatch = trimmed.match(/S:([01]{3})/) || trimmed.match(/([01]{3})/);

    const sensors = sMatch ? sMatch[1] : '000';
    const left = sensors[0] === '1' ? '1' : '0';
    const right = sensors[1] === '1' ? '1' : '0';
    const front = sensors[2] === '1' ? '1' : '0';
    const hasObstacle = front === '1' || left === '1' || right === '1';
    return {
      state: hasObstacle ? '¡OBSTÁCULO!' : 'LISTO',
      sensor: `${left}${right}${front}`,
      temp: tMatch ? parseFloat(tMatch[1]).toFixed(1) : '—',
      hum: hMatch ? parseInt(hMatch[1]) : '—'
    };
}

async function pollStatus() {
    try {
        const controller = new AbortController();
        const id = setTimeout(() => controller.abort(), 1200);
        const res = await fetch('/status', {cache: 'no-cache', signal: controller.signal});
        clearTimeout(id);
        if (!res.ok) throw new Error('no ok');
        const txt = await res.text();
        const parsed = parseStatus(txt);
        stateEl.textContent = parsed.state;
        stateEl.classList.toggle('obstacle', parsed.state.includes('OBSTÁCULO'));
        sensorEl.textContent = parsed.sensor;
        document.getElementById('temp').textContent = parsed.temp;
        document.getElementById('hum').textContent = parsed.hum;

        // update meters (assume temp range 0-50, hum 0-100)
        if (tMeter && parsed.temp !== '—') {
          const pct = Math.max(0, Math.min(100, (parsed.temp / 50) * 100));
          tMeter.style.width = pct + '%';
        }
        if (hMeter && parsed.hum !== '—') {
          const pct = Math.max(0, Math.min(100, parsed.hum));
          hMeter.style.width = pct + '%';
        }

        connText.textContent = 'conectado';
        connEl.setAttribute('aria-status','ok');
        isConnected = true;
    } catch (e) {
        connText.textContent = 'desconectando…';
        connEl.removeAttribute('aria-status');
        isConnected = false;
    }
}

// visual feedback on pointer
function setupControls() {
    buttons.forEach(btn => {
        btn.classList.add('ripple');
        const cmd = btn.dataset.cmd;
        let pointerActive = false;
        btn.addEventListener('pointerdown', (e) => {
            e.preventDefault();
            pointerActive = true;
            btn.classList.add('active');
            btn.setPointerCapture(e.pointerId);
            sendCmd(cmd);
        });
        btn.addEventListener('pointerup', (e) => {
            try { btn.releasePointerCapture(e.pointerId); } catch {}
            pointerActive = false;
            btn.classList.remove('active');
            sendCmd('stop');
        });
        btn.addEventListener('pointerleave', () => {
            if (pointerActive) {
              pointerActive = false;
              btn.classList.remove('active');
              sendCmd('stop');
            }
        });
        btn.addEventListener('pointercancel', () => { btn.classList.remove('active'); sendCmd('stop'); });
        if (cmd === 'stop') btn.addEventListener('click', () => sendCmd('stop'));
    });

    // Keyboard support
    const keyMap = { ArrowUp: 'forward', ArrowDown: 'backward', ArrowLeft: 'left', ArrowRight: 'right', Space: 'stop' };
    const activeKeys = new Set();
    window.addEventListener('keydown', (e) => {
        const code = e.code || e.key;
        if (keyMap[code] && !activeKeys.has(code)) {
            activeKeys.add(code);
            sendCmd(keyMap[code]);
            e.preventDefault();
        }
    });
    window.addEventListener('keyup', (e) => {
        const code = e.code || e.key;
        if (keyMap[code]) {
            activeKeys.delete(code);
            sendCmd('stop');
            e.preventDefault();
        }
    });
}

window.addEventListener('load', () => {
    setupControls();
    pollStatus();
    setInterval(pollStatus, 600); // ligero y responsivo
});