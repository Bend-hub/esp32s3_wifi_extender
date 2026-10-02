<script>
  let networks = [];
  let ssid = $state('');
  let scanning = false;
  let scanError = '';

  async function scanNetworks() {
    scanning = true;
    scanError = '';
    networks = [];

    try {
      const res = await fetch('/scan');
      if (!res.ok) {
        throw new Error(`Scan failed (${res.status})`);
      }

      const data = await res.json();

      // Expecting something like: { "networks": [{ "ssid": "MyWifi", "rssi": -55 }, ...] }
      networks = (data.networks ?? [])
        // drop any blank/hidden SSIDs
        .filter((n) => n.ssid && n.ssid.length > 0)
        // dedupe by ssid, keeping strongest signal
        .reduce((acc, n) => {
          const existing = acc.find((x) => x.ssid === n.ssid);
          if (!existing) {
            acc.push(n);
          } else if (n.rssi > existing.rssi) {
            Object.assign(existing, n);
          }
          return acc;
        }, [])
        // sort strongest signal first
        .sort((a, b) => b.rssi - a.rssi);

      if (networks.length === 0) {
        scanError = 'No networks found. Try scanning again.';
      } else if (!networks.some((n) => n.ssid === ssid)) {
        ssid = networks[0].ssid;
      }
    } catch (err) {
      scanError = err.message || 'Could not reach device to scan.';
    } finally {
      scanning = false;
    }
  }

  // simple bar count from rssi, purely cosmetic
  function signalBars(rssi) {
    if (rssi >= -55) return '▂▄▆█';
    if (rssi >= -70) return '▂▄▆';
    if (rssi >= -80) return '▂▄';
    return '▂';
  }
	
	import Scan from './Scan.svelte';
  let password = '';
  let submitting = false;
  let statusText = 'Not connected';
  let statusClass = 'offline';
  let errorMsg = '';
	

  async function handleSubmit() {
    errorMsg = '';
    submitting = true;
    statusText = 'Connecting...';
    statusClass = 'connecting';

    try {
      const res = await fetch('/connect', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ ssid, password })
      });

      if (!res.ok) {
        throw new Error(`Server responded with ${res.status}`);
      }

      // Give the device a moment to associate, then start polling
      pollConnectionStatus();
    } catch (err) {
      errorMsg = err.message || 'Failed to submit credentials';
      statusText = 'Connection failed';
      statusClass = 'offline';
    } finally {
      submitting = false;
    }
  }

  function pollConnectionStatus() {
    const interval = setInterval(async () => {
      try {
        const res = await fetch('/status');
        const data = await res.json();

        if (data.connected) {
          statusText = `Connected to ${data.ssid || ssid}`;
          statusClass = 'online';
          clearInterval(interval);
        } else {
          statusText = 'Connecting...';
          statusClass = 'connecting';
        }
      } catch (e) {
        // Device may briefly drop off network while switching
        statusText = 'Connecting...';
        statusClass = 'connecting';
      }
    }, 2000);
  }
</script>

<main>
  <div class="card">
    <h1>WiFi Setup</h1>
    <p class="subtitle">Connect your device to a wireless network</p>

    <div class="status status-{statusClass}">
      <span class="dot"></span>
      {statusText}
    </div>

		<label for="ssid-select">Scan For Networks</label>
		<div class="scan-row">
		  <select id="ssid-select" bind:value={ssid} disabled={scanning || networks.length === 0}>
		    {#if networks.length === 0}
		      <option value="" disabled selected>No networks scanned yet</option>
		    {/if}
		    {#each networks as net}
		      <option value={net.ssid}>
		        {net.ssid} {signalBars(net.rssi)}
		      </option>
		    {/each}
		  </select>
		
		  <button type="button" class="scan-btn" on:click={scanNetworks} disabled={scanning}>
		    {#if scanning}
		      Scanning…
		    {:else}
		      Scan
		    {/if}
		  </button>
		</div>

    <form on:submit|preventDefault={handleSubmit}>
      <label for="ssid">Network Name (SSID)</label>
      <input
        id="ssid"
        type="text"
        bind:value={ssid}
        placeholder="e.g. Home Network"
        required
      />

      <label for="password">Password</label>
      <input
        id="password"
        type="password"
        bind:value={password}
        placeholder="Network password"
      />

      {#if errorMsg}
        <p class="error">{errorMsg}</p>
      {/if}

      <button type="submit" disabled={submitting}>
        {submitting ? 'Connecting...' : 'Connect'}
      </button>
    </form>
  </div>
</main>

<style>
  .scan-row {
    display: flex;
    gap: 0.5rem;
    align-items: stretch;
  }

  select {
    flex: 1;
    border: 1px solid #bfdbfe;
    border-radius: 8px;
    padding: 0.65rem 0.75rem;
    font-size: 0.95rem;
    background: #f8fafc;
    color: #1e293b;
    outline: none;
    transition: border-color 0.15s, box-shadow 0.15s;
  }

  select:focus {
    border-color: #3b82f6;
    box-shadow: 0 0 0 3px rgba(59, 130, 246, 0.15);
    background: #ffffff;
  }

  select:disabled {
    color: #94a3b8;
    cursor: not-allowed;
  }

  .scan-btn {
    margin-top: 0;
    padding: 0.65rem 1rem;
    border: none;
    border-radius: 8px;
    background: #2563eb;
    color: #ffffff;
    font-size: 0.9rem;
    font-weight: 600;
    cursor: pointer;
    white-space: nowrap;
    transition: background 0.15s;
  }

  .scan-btn:hover:not(:disabled) {
    background: #1d4ed8;
  }

  .scan-btn:disabled {
    background: #93c5fd;
    cursor: not-allowed;
  }
  :global(body) {
    margin: 0;
    font-family: 'Segoe UI', system-ui, sans-serif;
    background: #eaf2fb;
  }

  main {
    min-height: 100vh;
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 1.5rem;
  }

  .card {
    background: #ffffff;
    border-radius: 16px;
    padding: 2.5rem;
    width: 100%;
    max-width: 380px;
    box-shadow: 0 10px 30px rgba(30, 90, 180, 0.12);
    border: 1px solid #dbeafe;
  }

  h1 {
    margin: 0 0 0.25rem;
    color: #1e3a8a;
    font-size: 1.6rem;
  }

  .subtitle {
    margin: 0 0 1.5rem;
    color: #5b7ba8;
    font-size: 0.9rem;
  }

  .status {
    display: flex;
    align-items: center;
    gap: 0.5rem;
    font-size: 0.85rem;
    font-weight: 600;
    padding: 0.6rem 0.9rem;
    border-radius: 8px;
    margin-bottom: 1.5rem;
    background: #f1f6fd;
    color: #2563eb;
  }

  .dot {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: #93c5fd;
  }

  .status-online {
    background: #eaf7ee;
    color: #16803d;
  }
  .status-online .dot {
    background: #22c55e;
  }

  .status-connecting .dot {
    background: #3b82f6;
    animation: pulse 1s infinite ease-in-out;
  }

  .status-offline .dot {
    background: #94a3b8;
  }

  @keyframes pulse {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.3; }
  }

  form {
    display: flex;
    flex-direction: column;
  }

  label {
    font-size: 0.8rem;
    font-weight: 600;
    color: #1e40af;
    margin-bottom: 0.35rem;
    margin-top: 0.9rem;
  }

  input {
    border: 1px solid #bfdbfe;
    border-radius: 8px;
    padding: 0.65rem 0.75rem;
    font-size: 0.95rem;
    background: #f8fafc;
    color: #1e293b;
    outline: none;
    transition: border-color 0.15s, box-shadow 0.15s;
  }

  input:focus {
    border-color: #3b82f6;
    box-shadow: 0 0 0 3px rgba(59, 130, 246, 0.15);
    background: #ffffff;
  }

  .error {
    color: #dc2626;
    font-size: 0.8rem;
    margin: 0.75rem 0 0;
  }

  button {
    margin-top: 1.75rem;
    padding: 0.75rem;
    border: none;
    border-radius: 8px;
    background: #2563eb;
    color: #ffffff;
    font-size: 0.95rem;
    font-weight: 600;
    cursor: pointer;
    transition: background 0.15s;
  }

  button:hover:not(:disabled) {
    background: #1d4ed8;
  }

  button:disabled {
    background: #93c5fd;
    cursor: not-allowed;
  }
</style>