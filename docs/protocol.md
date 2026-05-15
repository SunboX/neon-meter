# BLE Protocol

## Service

The firmware advertises as `Neon Meter` and exposes a custom BLE GATT service:

| Item | UUID |
| --- | --- |
| Data Service | `41494d45-7465-7220-0000-000000000001` |
| RX write characteristic | `41494d45-7465-7220-0000-000000000002` |
| TX notify/read characteristic | `41494d45-7465-7220-0000-000000000003` |
| Refresh notify characteristic | `41494d45-7465-7220-0000-000000000004` |

The UUIDs are stable for host compatibility.

The host writes a compact UTF-8 JSON payload to RX. The device notifies TX with
`{"ack":true}` or `{"err":true}`. When the host subscribes to the refresh
characteristic and the device has no data yet, the device sends a one-byte
notification to request a fresh payload.

## Provider Bundle

Current Neon Meter hosts send one bundle with every detected provider. If the
bundle contains one provider, the device keeps showing that provider. If it
contains two, the device rotates between cached provider screens using
`rotationSeconds`, which defaults to `30`.

```json
{
  "rotationSeconds": 30,
  "providers": [
    {
      "p": "claude",
      "title": "Claude Code",
      "s": 29,
      "sl": "Session",
      "sr": 142,
      "w": 4,
      "wl": "Weekly",
      "wr": 9730,
      "st": "allowed",
      "detail": "5h 29% / 7d 4%",
      "ok": true
    },
    {
      "p": "chatgpt",
      "title": "ChatGPT",
      "s": 55,
      "sl": "Session",
      "sr": 180,
      "w": 40,
      "wl": "Weekly",
      "wr": 10080,
      "st": "ok",
      "detail": "5h 55% / 7d 40%",
      "ok": true
    }
  ]
}
```

Single compact provider objects remain supported for compatibility.

## Payload Fields

| Field | Type | Meaning |
| --- | --- | --- |
| `p` | string | Provider key, such as `claude`, `chatgpt`, or `host`. Defaults to `claude` for Clawdmeter compatibility. |
| `title` | string | Optional UI title. Defaults from provider. |
| `s` | number | Primary/current usage percent, clamped to 0-100. |
| `sl` | string | Primary/current panel label. Defaults to `Current`. |
| `sr` | number | Minutes until primary/current window reset. `-1` means unknown. |
| `w` | number | Secondary/weekly/monthly usage percent, clamped to 0-100. |
| `wl` | string | Secondary panel label. Defaults to `Weekly`. |
| `wr` | number | Minutes until secondary window reset. `-1` means unknown. |
| `st` | string | Provider status, such as `ok`, `allowed`, `limited`, or `error`. |
| `detail` | string | Optional short status/spend note. |
| `ok` | boolean | Whether the host-side fetch succeeded. |

## Examples

Claude-compatible:

```json
{"s":29,"sr":142,"w":4,"wr":9730,"st":"allowed","ok":true}
```

ChatGPT/Codex:

```json
{"p":"chatgpt","title":"ChatGPT","s":55,"sl":"Session","sr":180,"w":40,"wl":"Weekly","wr":10080,"st":"ok","detail":"5h 55% / 7d 40%","ok":true}
```
