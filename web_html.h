#pragma once

// web_html.h — Dashboard Smart Pump (UI redesign: ControlX-style)

static const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Smart Pump — Cavitation Monitor</title>
<style>
:root{
  --bg:#070b16;--bg2:#0b1120;--card:#111a2e;--card2:#0d1525;--inset:#0a1120;
  --bd:#1e2a44;--bd2:#27344f;--txt:#e6edf6;--dim:#6b7e99;--dim2:#9fb0c8;
  --pri:#6366f1;--pri2:#8b5cf6;--cyan:#22d3ee;--cyan2:#0ea5e9;
  --grn:#10b981;--grn2:#34d399;--red:#f43f5e;--red2:#ef4444;
  --amb:#f59e0b;--amb2:#fbbf24;--pur:#a855f7;--ora:#f97316;
  --r:14px;--r2:10px;--sh:0 6px 24px rgba(0,0,0,.45);
}
*{box-sizing:border-box;margin:0;padding:0}
body{
  background:
    radial-gradient(900px 500px at 85% -8%,rgba(99,102,241,.10),transparent 60%),
    radial-gradient(700px 460px at -10% 4%,rgba(34,211,238,.07),transparent 55%),
    var(--bg);
  color:var(--txt);font-family:'Segoe UI',system-ui,-apple-system,sans-serif;
  padding:62px 12px 18px;font-size:14px;-webkit-font-smoothing:antialiased;min-height:100vh}
.mono{font-family:'SF Mono','Cascadia Code',ui-monospace,Menlo,monospace}

/* ── Top bar ─────────────────────────────────────────────── */
.topbar{position:fixed;top:0;left:0;right:0;z-index:200;height:54px;display:flex;align-items:center;gap:10px;
  padding:0 12px;background:rgba(9,13,24,.86);backdrop-filter:blur(12px);border-bottom:1px solid var(--bd)}
.brand{display:flex;align-items:center;gap:9px;white-space:nowrap}
.brand-logo{width:30px;height:30px;border-radius:9px;background:linear-gradient(135deg,var(--pri),var(--pri2));
  display:flex;align-items:center;justify-content:center;box-shadow:0 4px 14px rgba(99,102,241,.5)}
.brand-logo svg{width:17px;height:17px;stroke:#fff;fill:none;stroke-width:2}
.brand-txt{font-weight:700;font-size:.92rem;letter-spacing:.01em;line-height:1}
.brand-txt span{display:block;font-size:.56rem;font-weight:500;color:var(--dim);letter-spacing:.16em;margin-top:2px}
.tabs{display:flex;gap:2px;flex:1;overflow-x:auto;scrollbar-width:none;margin:0 4px}
.tabs::-webkit-scrollbar{display:none}
.tab{background:transparent;border:none;color:var(--dim);font-size:.72rem;font-weight:600;cursor:pointer;
  white-space:nowrap;padding:8px 13px;border-radius:9px;transition:.16s;flex-shrink:0;letter-spacing:.01em}
.tab:hover{color:var(--dim2);background:rgba(255,255,255,.04)}
.tab.active{color:#fff;background:linear-gradient(135deg,var(--pri),var(--pri2));box-shadow:0 4px 14px rgba(99,102,241,.4)}
.bar-actions{display:flex;align-items:center;gap:7px;flex-shrink:0}
.icon-btn{position:relative;width:36px;height:36px;border-radius:10px;border:1px solid var(--bd);background:var(--card2);
  color:var(--dim2);cursor:pointer;display:flex;align-items:center;justify-content:center;transition:.16s}
.icon-btn:hover{border-color:var(--bd2);color:#fff;background:var(--card)}
.icon-btn.on{border-color:var(--pri);color:#c7d2fe}
.icon-btn svg{width:18px;height:18px;stroke:currentColor;fill:none;stroke-width:1.8}
.notif-badge{position:absolute;top:-4px;right:-4px;min-width:17px;height:17px;padding:0 4px;border-radius:9px;
  background:var(--red);color:#fff;font-size:.56rem;font-weight:800;display:none;align-items:center;justify-content:center;
  border:2px solid var(--bg);font-family:system-ui}
.notif-badge.show{display:flex}

/* ── Dropdown panels (notif + visibility) ─────────────────── */
.pop{position:fixed;top:60px;right:12px;z-index:210;width:300px;max-width:calc(100vw - 24px);
  background:var(--card);border:1px solid var(--bd2);border-radius:var(--r);box-shadow:var(--sh);
  opacity:0;transform:translateY(-8px) scale(.98);pointer-events:none;transition:.18s}
.pop.open{opacity:1;transform:none;pointer-events:auto}
.pop-h{display:flex;justify-content:space-between;align-items:center;padding:13px 15px;border-bottom:1px solid var(--bd)}
.pop-h b{font-size:.82rem;font-weight:700}
.pop-h a{font-size:.66rem;color:var(--pri2);cursor:pointer;text-decoration:none}
.pop-h a:hover{text-decoration:underline}
.notif-list{max-height:340px;overflow-y:auto}
.notif-item{display:flex;gap:11px;padding:11px 15px;border-bottom:1px solid rgba(30,42,68,.5);transition:.15s}
.notif-item:hover{background:rgba(255,255,255,.025)}
.notif-item.unread{background:rgba(99,102,241,.06)}
.ni-ico{width:30px;height:30px;border-radius:9px;flex-shrink:0;display:flex;align-items:center;justify-content:center}
.ni-ico svg{width:15px;height:15px;stroke:#fff;fill:none;stroke-width:2}
.ni-ico.crit{background:linear-gradient(135deg,#b91c3c,var(--red))}
.ni-ico.warn{background:linear-gradient(135deg,#b45309,var(--amb))}
.ni-ico.ok{background:linear-gradient(135deg,#047857,var(--grn))}
.ni-ico.info{background:linear-gradient(135deg,#0369a1,var(--cyan2))}
.ni-body{flex:1;min-width:0}
.ni-title{font-size:.74rem;font-weight:600;line-height:1.25}
.ni-msg{font-size:.66rem;color:var(--dim2);margin-top:2px;line-height:1.35}
.ni-time{font-size:.58rem;color:var(--dim);margin-top:3px}
.notif-empty{padding:34px 16px;text-align:center;color:var(--dim);font-size:.72rem}
.vis-item{display:flex;align-items:center;gap:10px;padding:9px 15px;cursor:pointer;border-bottom:1px solid rgba(30,42,68,.4)}
.vis-item:hover{background:rgba(255,255,255,.03)}
.vis-item input{accent-color:var(--pri);cursor:pointer;width:15px;height:15px}
.vis-item label{font-size:.72rem;color:var(--dim2);cursor:pointer;flex:1}
.vis-sep{font-size:.6rem;font-weight:700;color:var(--dim);text-transform:uppercase;letter-spacing:.1em;padding:9px 15px 5px;border-top:1px solid var(--bd);margin-top:2px}

/* ── Status strip ─────────────────────────────────────────── */
.status-strip{display:flex;gap:9px;align-items:stretch;margin-bottom:13px;flex-wrap:wrap}
#status{flex:1;padding:11px 14px;border-radius:var(--r2);border:1px solid var(--bd);font-weight:700;font-size:.74rem;
  letter-spacing:.1em;display:flex;align-items:center;justify-content:center;gap:8px;min-width:130px;background:var(--card2)}
#status::before{content:'';width:9px;height:9px;border-radius:50%;background:currentColor;box-shadow:0 0 10px currentColor}
#status.run{color:var(--grn2);border-color:rgba(16,185,129,.4);background:rgba(16,185,129,.07)}
#status.stp{color:var(--red);border-color:rgba(244,63,94,.4);background:rgba(244,63,94,.07)}
#status.off{color:var(--dim);border-color:var(--bd)}
.sensor-box{flex:1;padding:11px 14px;border-radius:var(--r2);border:1px solid rgba(16,185,129,.35);
  background:rgba(16,185,129,.06);color:var(--grn2);font-size:.74rem;font-weight:700;text-align:center;min-width:130px;
  display:flex;align-items:center;justify-content:center}

/* ── Pages & headers ──────────────────────────────────────── */
.page{display:none;animation:fade .35s ease}
.page.active{display:block}
@keyframes fade{from{opacity:0;transform:translateY(6px)}to{opacity:1;transform:none}}
.pg-hdr{display:flex;justify-content:space-between;align-items:flex-end;margin-bottom:14px}
.pg-hdr h1{font-size:1.25rem;font-weight:700;letter-spacing:-.01em}
.pg-hdr h1 b{font-weight:700;background:linear-gradient(135deg,var(--cyan),var(--pri2));-webkit-background-clip:text;background-clip:text;-webkit-text-fill-color:transparent}
.pg-hdr .sub{font-size:.66rem;color:var(--dim);margin-top:3px}
.pg-hdr .ts{font-size:.62rem;color:var(--dim)}

/* ── Generic card ─────────────────────────────────────────── */
.card{background:linear-gradient(180deg,var(--card),var(--card2));border:1px solid var(--bd);border-radius:var(--r);
  padding:15px 16px;margin-bottom:13px;box-shadow:var(--sh)}
.card-h{display:flex;justify-content:space-between;align-items:center;gap:8px;margin-bottom:13px;flex-wrap:wrap}
.card-h .t{display:flex;align-items:center;gap:9px;font-size:.78rem;font-weight:700}
.card-h .t .ic{width:26px;height:26px;border-radius:8px;display:flex;align-items:center;justify-content:center;flex-shrink:0}
.card-h .t .ic svg{width:15px;height:15px;stroke:#fff;fill:none;stroke-width:2}
.badge{font-size:.6rem;font-weight:700;padding:4px 9px;border-radius:7px;letter-spacing:.04em;font-family:system-ui}
.badge.ok{background:rgba(16,185,129,.14);color:var(--grn2)}
.badge.def{background:rgba(107,126,153,.14);color:var(--dim2)}
.badge.err{background:rgba(244,63,94,.14);color:var(--red)}
.badge.warn{background:rgba(245,158,11,.14);color:var(--amb2)}
.badge.pill{background:rgba(99,102,241,.14);color:#c7d2fe}
.info-box{background:var(--inset);border:1px solid var(--bd);border-radius:var(--r2);padding:12px 14px;margin-bottom:13px;
  font-size:.68rem;color:var(--dim2);line-height:1.7}
.info-box b{color:var(--txt)}

/* ── Buttons ──────────────────────────────────────────────── */
.btns{display:flex;gap:10px;margin-bottom:13px}
.btn{flex:1;padding:13px;border:none;border-radius:var(--r2);cursor:pointer;font-weight:700;font-size:.78rem;
  letter-spacing:.04em;color:#fff;display:flex;align-items:center;justify-content:center;gap:7px;transition:.16s;box-shadow:var(--sh)}
.btn:active{transform:translateY(1px)}
.btn svg{width:15px;height:15px;fill:currentColor}
.btn-s{background:linear-gradient(135deg,var(--grn),#059669)}
.btn-x{background:linear-gradient(135deg,var(--red),#be123c)}

/* ── Stat cards (KPI) ─────────────────────────────────────── */
.statgrid{display:grid;grid-template-columns:1fr 1fr;gap:11px;margin-bottom:13px}
.stat{background:linear-gradient(180deg,var(--card),var(--card2));border:1px solid var(--bd);border-radius:var(--r);
  padding:14px;position:relative;overflow:hidden;transition:.25s;box-shadow:var(--sh)}
.stat:hover{border-color:var(--bd2);transform:translateY(-2px)}
.stat.disabled{opacity:.4;filter:grayscale(.4)}
.stat-top{display:flex;justify-content:space-between;align-items:flex-start}
.stat-ico{width:42px;height:42px;border-radius:12px;display:flex;align-items:center;justify-content:center;box-shadow:0 6px 16px rgba(0,0,0,.4)}
.stat-ico svg{width:22px;height:22px;stroke:#fff;fill:none;stroke-width:1.9}
.stat-ico.cyan{background:linear-gradient(135deg,var(--cyan),var(--cyan2))}
.stat-ico.pur{background:linear-gradient(135deg,var(--pur),var(--pri)) }
.stat-ico.grn{background:linear-gradient(135deg,var(--grn2),var(--grn))}
.stat-ico.amb{background:linear-gradient(135deg,var(--amb2),var(--ora))}
.stat-lbl{font-size:.62rem;color:var(--dim);text-transform:uppercase;letter-spacing:.07em;margin-top:13px}
.stat-val{font-size:1.7rem;font-weight:700;line-height:1.05;margin-top:5px;letter-spacing:-.02em}
.stat-unit{font-size:.7rem;color:var(--dim2);font-weight:600;margin-left:4px}
.stat-sub{font-size:.6rem;color:var(--dim);margin-top:5px}
.stat-sub b{color:var(--dim2);font-weight:600}

/* ── Two-column row (cav ring + pid) ──────────────────────── */
.row2{display:grid;grid-template-columns:1fr 1fr;gap:11px;margin-bottom:13px}
@media(max-width:560px){.row2{grid-template-columns:1fr}.statgrid{grid-template-columns:1fr 1fr}}

/* Cavitation ring gauge */
.cav{background:linear-gradient(180deg,var(--card),var(--card2));border:1px solid var(--bd);border-radius:var(--r);
  padding:15px;box-shadow:var(--sh);transition:border-color .3s}
.cav.n{border-color:rgba(16,185,129,.45)}.cav.e{border-color:rgba(245,158,11,.5)}.cav.s{border-color:rgba(244,63,94,.55)}
.cav-lbl{font-size:.62rem;color:var(--dim);text-transform:uppercase;letter-spacing:.07em;text-align:center}
.ring-wrap{position:relative;width:138px;height:138px;margin:8px auto 6px}
.ring-wrap svg{transform:rotate(-90deg)}
.ring-c{position:absolute;inset:0;display:flex;flex-direction:column;align-items:center;justify-content:center}
.cav-val{font-size:1.85rem;font-weight:700;font-family:'SF Mono',ui-monospace,monospace;line-height:1}
.ring-c small{font-size:.52rem;color:var(--dim);text-transform:uppercase;letter-spacing:.1em;margin-top:3px}
.cav-st{display:block;margin:0 auto;width:fit-content;font-size:.62rem;font-weight:700;letter-spacing:.05em;padding:5px 12px;border-radius:8px}
.cav-st.n{background:rgba(16,185,129,.14);color:var(--grn2)}
.cav-st.e{background:rgba(245,158,11,.14);color:var(--amb2)}
.cav-st.s{background:rgba(244,63,94,.14);color:var(--red)}

/* PID panel */
.pid-wrap{background:linear-gradient(180deg,var(--card),var(--card2));border:1px solid var(--bd);border-radius:var(--r);padding:15px;box-shadow:var(--sh)}
.pid-top{display:flex;justify-content:space-between;align-items:center;margin-bottom:12px;flex-wrap:wrap;gap:6px}
.pid-top .lbl{font-size:.62rem;color:var(--dim);text-transform:uppercase;letter-spacing:.07em}
.pid-sp{font-size:.58rem;color:var(--dim);font-family:'SF Mono',ui-monospace,monospace}
.pid-mode-tag{display:inline-block;font-size:.56rem;font-weight:700;border-radius:6px;padding:3px 8px;margin-left:7px;letter-spacing:.04em}
.pid-mode-tag.V{background:rgba(16,185,129,.16);color:var(--grn2)}
.pid-mode-tag.P{background:rgba(34,211,238,.16);color:var(--cyan)}
.pid-mode-tag.I{background:rgba(168,85,247,.16);color:var(--pur)}
.pid-row{display:flex;align-items:center;gap:9px;margin:9px 0;font-size:.68rem}
.pid-lbl{width:78px;color:var(--dim)}
.pid-track{flex:1;background:var(--inset);border-radius:5px;height:11px;overflow:hidden;border:1px solid var(--bd)}
.pid-fill{height:100%;border-radius:5px;transition:.3s;background:var(--grn)}
.pid-num{width:62px;text-align:right;font-family:'SF Mono',ui-monospace,monospace;font-size:.7rem;color:var(--cyan)}

/* ── Sensor toggles ───────────────────────────────────────── */
.sens-card{background:linear-gradient(180deg,var(--card),var(--card2));border:1px solid var(--bd);border-radius:var(--r);padding:15px;margin-bottom:13px;box-shadow:var(--sh)}
.sens-title{display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:6px;font-size:.78rem;font-weight:700;margin-bottom:13px}
.sens-mode{font-family:'SF Mono',ui-monospace,monospace;font-size:.62rem;color:#c7d2fe;background:rgba(99,102,241,.13);padding:4px 10px;border-radius:7px}
.sens-row{display:grid;grid-template-columns:1fr 1fr 1fr;gap:9px}
.sens-item{background:var(--inset);border:1px solid var(--bd);border-radius:var(--r2);padding:12px 8px;text-align:center;transition:.3s}
.sens-item.on{border-color:rgba(16,185,129,.45)}.sens-item.off{border-color:rgba(244,63,94,.35);opacity:.6}
.sens-name{font-size:.58rem;letter-spacing:.06em;text-transform:uppercase;margin-bottom:9px;font-weight:600}
.sens-name.on{color:var(--grn2)}.sens-name.off{color:var(--red)}
.tog{position:relative;display:inline-block;width:48px;height:26px;cursor:pointer}
.tog input{display:none}
.tog-slider{position:absolute;inset:0;background:#2a3650;border-radius:26px;transition:.3s}
.tog-slider:before{content:"";position:absolute;height:20px;width:20px;left:3px;top:3px;background:#7e8aa0;border-radius:50%;transition:.3s}
.tog input:checked+.tog-slider{background:linear-gradient(135deg,var(--grn),#059669)}
.tog input:checked+.tog-slider:before{transform:translateX(22px);background:#fff}
.sens-status{font-size:.58rem;font-family:'SF Mono',ui-monospace,monospace;margin-top:6px;font-weight:600}
.mini-grid{display:grid;gap:9px;margin-bottom:11px}
.mini{background:var(--inset);border:1px solid var(--bd);border-radius:var(--r2);padding:10px 12px}
.mini-lbl{font-size:.56rem;color:var(--dim);text-transform:uppercase;letter-spacing:.05em}
.mini-val{font-family:'SF Mono',ui-monospace,monospace;font-size:1.2rem;margin-top:3px;font-weight:600}
.mini-sub{font-size:.54rem;color:var(--dim);margin-top:2px;font-family:'SF Mono',ui-monospace,monospace}
input[type=range]{flex:1;accent-color:var(--pur);height:5px}
.sld-row{display:flex;align-items:center;gap:10px;margin-bottom:11px}
.sld-v{font-family:'SF Mono',ui-monospace,monospace;font-size:.92rem;min-width:34px;text-align:right;font-weight:700}
.help{font-size:.6rem;color:var(--dim);line-height:1.6}
.help b{color:var(--dim2)}

/* ── MF card ──────────────────────────────────────────────── */
.mf-card{background:linear-gradient(180deg,var(--card),var(--card2));border:1px solid var(--bd);border-radius:var(--r);padding:15px;margin-bottom:13px;box-shadow:var(--sh)}
.mf-weights{font-family:'SF Mono',ui-monospace,monospace;font-size:.6rem;color:var(--dim);background:var(--inset);padding:4px 10px;border-radius:7px}
.mf-weights b{color:var(--grn2)}
.mf-row{display:grid;grid-template-columns:80px 1fr 92px;gap:9px;align-items:center;margin-bottom:9px;padding:9px;background:var(--inset);border-radius:var(--r2);transition:.3s;border:1px solid var(--bd)}
.mf-row.disabled{opacity:.38}
.mf-name{font-size:.6rem;color:var(--dim);text-transform:uppercase;letter-spacing:.05em}
.mf-name b{color:var(--cyan);display:block;font-size:.76rem;letter-spacing:.02em}
.mf-svg{width:100%;height:60px;display:block}
.mf-val{text-align:right;font-family:'SF Mono',ui-monospace,monospace;font-size:.68rem;color:var(--cyan)}
.mf-val b{display:block;font-size:.98rem;color:#fff}
.mf-val span{font-size:.52rem;color:var(--dim)}
.mf-legend{display:flex;gap:12px;justify-content:center;font-size:.55rem;font-family:'SF Mono',ui-monospace,monospace;color:var(--dim);margin-top:10px;flex-wrap:wrap}
.mf-legend span{display:flex;align-items:center;gap:4px}
.mf-legend span::before{content:"";width:11px;height:3px;border-radius:2px;display:inline-block}
.mf-legend .lo::before{background:#3b82f6}
.mf-legend .no::before{background:var(--grn)}
.mf-legend .hi::before{background:var(--red2)}
.mf-legend .sp::before{background:var(--amb);height:9px;width:2px}
.mf-legend .pv::before{background:#fff;height:9px;width:2px}

/* ── Calibration cards ────────────────────────────────────── */
.xinfo{display:grid;grid-template-columns:1fr 1fr;gap:9px;margin-bottom:11px}
.xinfo-item{background:var(--inset);border:1px solid var(--bd);border-radius:var(--r2);padding:9px 11px}
.xinfo-lbl{font-size:.54rem;color:var(--dim);text-transform:uppercase;letter-spacing:.05em}
.xinfo-val{font-family:'SF Mono',ui-monospace,monospace;font-size:.92rem;font-weight:600;margin-top:3px}
.xinfo-val.pur{color:var(--pur)}.xinfo-val.grn{color:var(--grn2)}.xinfo-val.amb{color:var(--amb2)}
.xbtns{display:flex;gap:9px;margin-bottom:9px;flex-wrap:wrap}
.xbtn{flex:1;padding:11px 8px;border:none;border-radius:var(--r2);cursor:pointer;font-weight:700;font-size:.68rem;
  letter-spacing:.03em;color:#fff;min-width:90px;transition:.16s}
.xbtn:active{transform:translateY(1px)}
.xbtn-pri{background:linear-gradient(135deg,var(--pur),var(--pri))}
.xbtn-pri2{background:linear-gradient(135deg,var(--pri),var(--cyan2))}
.xbtn-grn{background:linear-gradient(135deg,var(--grn),#059669)}
.xbtn-amb{background:linear-gradient(135deg,var(--amb),var(--ora))}
.xbtn-rst{background:linear-gradient(135deg,#3a4763,#566184)}
.xspan-row{display:flex;gap:9px;align-items:center;margin-bottom:9px;font-size:.66rem;color:var(--dim);background:var(--inset);padding:9px 11px;border-radius:var(--r2);flex-wrap:wrap}
.xspan-row input{flex:1;min-width:90px;background:var(--card2);border:1px solid var(--bd2);border-radius:7px;
  color:var(--txt);padding:7px 9px;font-size:.78rem;font-family:'SF Mono',ui-monospace,monospace;text-align:center}
.xspan-row input.pur{color:var(--pur)}.xspan-row input.amb{color:var(--amb2)}
.xhelp{font-size:.58rem;color:var(--dim);line-height:1.7;background:var(--inset);padding:10px 12px;border-radius:var(--r2)}
.xhelp.pur{border-left:3px solid var(--pur)}.xhelp.grn{border-left:3px solid var(--grn)}.xhelp.amb{border-left:3px solid var(--amb)}
.xhelp b{display:block;margin-bottom:3px}.xhelp b.pur{color:var(--pur)}.xhelp b.grn{color:var(--grn2)}.xhelp b.amb{color:var(--amb2)}
.xmsg{font-size:.64rem;font-family:'SF Mono',ui-monospace,monospace;padding:7px 11px;border-radius:7px;margin-top:7px;display:none}
.xmsg.show{display:block}
.xmsg.ok{background:rgba(16,185,129,.12);color:var(--grn2)}
.xmsg.err{background:rgba(244,63,94,.12);color:var(--red)}
.xprog{background:var(--inset);border-radius:4px;height:5px;overflow:hidden;margin-top:7px;display:none}
.xprog.show{display:block}
.xprog-fill{height:100%;border-radius:4px;transition:.3s}

/* ── Chart ────────────────────────────────────────────────── */
.chart-card{background:linear-gradient(180deg,var(--card),var(--card2));border:1px solid var(--bd);border-radius:var(--r);padding:15px;margin-bottom:13px;box-shadow:var(--sh)}
.chart-info{font-family:'SF Mono',ui-monospace,monospace;font-size:.58rem;color:var(--dim);background:var(--inset);padding:4px 10px;border-radius:7px}
.chart-info b{color:var(--cyan)}
#trendCanvas{width:100%;height:230px;display:block;background:#0a1322;border:1px solid var(--bd);border-radius:var(--r2);cursor:crosshair;margin-bottom:10px}
.chart-legend{display:flex;gap:8px;justify-content:center;flex-wrap:wrap;margin-bottom:11px}
.chart-leg{display:flex;align-items:center;gap:6px;font-size:.62rem;font-family:'SF Mono',ui-monospace,monospace;color:var(--dim2);
  background:var(--inset);padding:5px 10px;border-radius:8px;cursor:pointer;user-select:none;transition:.18s;border:1px solid var(--bd)}
.chart-leg:hover{border-color:var(--bd2)}
.chart-leg.off{opacity:.35;text-decoration:line-through}
.chart-leg .dot{width:9px;height:9px;border-radius:50%}
.chart-leg .lbl{display:flex;flex-direction:column;line-height:1.1}
.chart-leg .lbl span{font-size:.5rem;color:var(--dim)}
.chart-btns{display:flex;gap:9px;flex-wrap:wrap}
.chart-btn{flex:1;padding:10px;border:none;border-radius:var(--r2);cursor:pointer;font-weight:700;font-size:.66rem;
  letter-spacing:.03em;color:#fff;min-width:130px;transition:.16s}
.chart-btn:active{transform:translateY(1px)}
.chart-btn-png{background:linear-gradient(135deg,var(--cyan2),var(--cyan))}
.chart-btn-ui{background:linear-gradient(135deg,var(--pur),var(--pri))}
.chart-btn-clr{background:linear-gradient(135deg,#3a4763,#566184)}

/* ── System monitor ───────────────────────────────────────── */
.tm-card{background:linear-gradient(180deg,var(--card),var(--card2));border:1px solid var(--bd);border-radius:var(--r);padding:15px;margin-bottom:13px;box-shadow:var(--sh);transition:border-color .3s}
.tm-card.warn{border-color:rgba(245,158,11,.5)}.tm-card.crit{border-color:rgba(244,63,94,.55)}
.tm-grid{display:grid;grid-template-columns:repeat(3,1fr);gap:9px}
@media(max-width:480px){.tm-grid{grid-template-columns:1fr 1fr}}
.tm-item{background:var(--inset);border:1px solid var(--bd);border-radius:var(--r2);padding:11px 10px;text-align:center}
.tm-item-lbl{font-size:.54rem;color:var(--dim);letter-spacing:.05em;text-transform:uppercase}
.tm-item-val{font-family:'SF Mono',ui-monospace,monospace;font-size:1.05rem;margin-top:4px;font-weight:700}
.tm-item-sub{font-size:.5rem;color:var(--dim);margin-top:2px;font-family:'SF Mono',ui-monospace,monospace}

/* ── Logger ───────────────────────────────────────────────── */
.log-int-row{display:flex;align-items:center;gap:10px;font-size:.68rem;color:var(--dim);margin-bottom:11px}
.log-int-row input{background:var(--card2);border:1px solid var(--bd2);border-radius:7px;color:var(--txt);
  padding:7px 9px;font-size:.78rem;font-family:'SF Mono',ui-monospace,monospace;width:90px;text-align:center}
.log-btns{display:flex;gap:9px;margin-bottom:13px;flex-wrap:wrap}
.lbtn{flex:1;padding:11px 8px;border:none;border-radius:var(--r2);cursor:pointer;font-weight:700;font-size:.7rem;
  letter-spacing:.03em;color:#fff;min-width:74px;transition:.16s}
.lbtn:active{transform:translateY(1px)}
.lbtn:disabled{opacity:.35;cursor:not-allowed}
.lb-start{background:linear-gradient(135deg,var(--pur),var(--pri))}
.lb-stop{background:linear-gradient(135deg,var(--red),#be123c)}
.lb-dl{background:linear-gradient(135deg,var(--cyan2),var(--cyan))}
.lb-clear{background:linear-gradient(135deg,#3a4763,#566184)}
.log-stats{display:grid;grid-template-columns:1fr 1fr;gap:9px;margin-bottom:11px}
.log-stat{background:var(--inset);border:1px solid var(--bd);border-radius:var(--r2);padding:10px 12px}
.log-stat-lbl{font-size:.56rem;color:var(--dim);text-transform:uppercase}
.log-stat-val{font-family:'SF Mono',ui-monospace,monospace;font-size:.92rem;color:var(--pur);font-weight:700;margin-top:3px}
.log-dot{display:inline-block;width:9px;height:9px;border-radius:50%;background:var(--dim);margin-right:7px;vertical-align:middle}
.log-dot.active{background:var(--red);animation:blink 1s infinite;box-shadow:0 0 8px var(--red)}
@keyframes blink{0%,100%{opacity:1}50%{opacity:.25}}
.log-prog-track{background:var(--inset);border:1px solid var(--bd);border-radius:4px;height:7px;overflow:hidden;margin-top:5px}
.log-prog-fill{height:100%;border-radius:4px;background:linear-gradient(90deg,var(--pri),var(--pur));transition:.5s}
.prog-cap{font-size:.6rem;color:var(--dim);display:flex;justify-content:space-between;margin-bottom:5px}

/* ── Modal (popup) ────────────────────────────────────────── */
.modal-overlay{position:fixed;inset:0;z-index:400;background:rgba(4,7,14,.72);backdrop-filter:blur(4px);
  display:flex;align-items:center;justify-content:center;padding:18px;opacity:0;pointer-events:none;transition:.2s}
.modal-overlay.open{opacity:1;pointer-events:auto}
.modal{width:340px;max-width:100%;background:linear-gradient(180deg,var(--card),var(--card2));border:1px solid var(--bd2);
  border-radius:18px;padding:24px;text-align:center;box-shadow:0 24px 60px rgba(0,0,0,.6);transform:scale(.92);transition:.2s}
.modal-overlay.open .modal{transform:none}
.modal-ico{width:60px;height:60px;border-radius:16px;margin:0 auto 16px;display:flex;align-items:center;justify-content:center}
.modal-ico svg{width:30px;height:30px;stroke:#fff;fill:none;stroke-width:2}
.modal-ico.warn{background:linear-gradient(135deg,var(--amb),var(--ora))}
.modal-ico.danger{background:linear-gradient(135deg,var(--red),#be123c)}
.modal-ico.ok{background:linear-gradient(135deg,var(--grn2),var(--grn))}
.modal-ico.info{background:linear-gradient(135deg,var(--cyan),var(--cyan2))}
.modal-title{font-size:1.02rem;font-weight:700;margin-bottom:8px}
.modal-msg{font-size:.76rem;color:var(--dim2);line-height:1.6;margin-bottom:20px;white-space:pre-line}
.modal-msg b{color:var(--txt)}
.modal-btns{display:flex;gap:10px}
.mbtn{flex:1;padding:12px;border:none;border-radius:var(--r2);cursor:pointer;font-weight:700;font-size:.78rem;transition:.16s}
.mbtn:active{transform:translateY(1px)}
.mbtn-cancel{background:var(--card2);border:1px solid var(--bd2);color:var(--dim2)}
.mbtn-cancel:hover{color:#fff;border-color:var(--bd2)}
.mbtn-confirm{background:linear-gradient(135deg,var(--pri),var(--pri2));color:#fff}
.mbtn-danger{background:linear-gradient(135deg,var(--red),#be123c);color:#fff}

/* ── Toast ────────────────────────────────────────────────── */
#toastWrap{position:fixed;bottom:18px;left:50%;transform:translateX(-50%);z-index:500;display:flex;flex-direction:column;gap:8px;align-items:center;width:calc(100% - 24px);max-width:380px}
.toast{width:100%;background:var(--card);border:1px solid var(--bd2);border-left:3px solid var(--pri);border-radius:var(--r2);
  padding:11px 14px;font-size:.72rem;color:var(--txt);box-shadow:var(--sh);display:flex;align-items:center;gap:9px;
  animation:toastIn .25s ease}
.toast.ok{border-left-color:var(--grn)}.toast.err{border-left-color:var(--red)}.toast.warn{border-left-color:var(--amb)}
.toast.hide{animation:toastOut .25s ease forwards}
@keyframes toastIn{from{opacity:0;transform:translateY(12px)}to{opacity:1;transform:none}}
@keyframes toastOut{to{opacity:0;transform:translateY(12px)}}
</style>
</head>
<body>

<!-- ═══ TOP BAR ═══════════════════════════════════════════════════════ -->
<header class="topbar">
  <div class="brand">
    <div class="brand-logo"><svg viewBox="0 0 24 24"><path d="M12 2v6m0 0a5 5 0 1 0 5 5"/><circle cx="12" cy="13" r="2"/></svg></div>
    <div class="brand-txt">SmartPump<span>CAVITATION MONITOR</span></div>
  </div>
  <nav class="tabs">
    <button class="tab active" data-page="beranda" onclick="showPage('beranda')">Beranda</button>
    <button class="tab" data-page="fuzzy" onclick="showPage('fuzzy')">Fuzzy MF</button>
    <button class="tab" data-page="kontrol" onclick="showPage('kontrol')">Kontrol</button>
    <button class="tab" data-page="kalibrasi" onclick="showPage('kalibrasi')">Kalibrasi</button>
    <button class="tab" data-page="logger" onclick="showPage('logger')">Logger</button>
    <button class="tab" data-page="sistem" onclick="showPage('sistem')">Sistem</button>
  </nav>
  <div class="bar-actions">
    <button class="icon-btn" id="notifBtn" onclick="toggleNotif()" title="Notifikasi">
      <svg viewBox="0 0 24 24"><path d="M18 8a6 6 0 0 0-12 0c0 7-3 9-3 9h18s-3-2-3-9"/><path d="M13.7 21a2 2 0 0 1-3.4 0"/></svg>
      <span class="notif-badge" id="notifBadge">0</span>
    </button>
    <button class="icon-btn" id="navVisBtn" onclick="toggleVisPanel()" title="Tampilkan panel">
      <svg viewBox="0 0 24 24"><rect x="3" y="3" width="7" height="7" rx="1"/><rect x="14" y="3" width="7" height="7" rx="1"/><rect x="3" y="14" width="7" height="7" rx="1"/><rect x="14" y="14" width="7" height="7" rx="1"/></svg>
    </button>
  </div>
</header>

<!-- Notif dropdown -->
<div class="pop" id="notifPanel">
  <div class="pop-h"><b>Notifikasi</b><a onclick="markAllRead()">Tandai sudah dibaca</a></div>
  <div class="notif-list" id="notifList"></div>
  <div class="notif-empty" id="notifEmpty">Belum ada notifikasi</div>
</div>

<!-- Visibility dropdown -->
<div class="pop" id="visPanel">
  <div class="pop-h"><b>Tampilkan panel</b><a onclick="showAllPanels()">Tampilkan semua</a></div>
  <div id="visPanelItems"></div>
</div>

<!-- Status strip -->
<div class="status-strip">
  <div id="status" class="off">MENGHUBUNGKAN…</div>
  <div id="sensorBox" class="sensor-box">SENSOR NORMAL</div>
</div>

<!-- ═══ BERANDA ═══════════════════════════════════════════════════════ -->
<div id="page-beranda" class="page active">
<div class="pg-hdr"><div><h1>Monitor <b>Utama</b></h1><div class="sub">Telemetri real-time deteksi kavitasi pompa</div></div><span class="ts" id="chartLastUpd">—</span></div>

<div id="pnl-ctrl">
  <div class="btns">
    <button class="btn btn-s" onclick="doStart()"><svg viewBox="0 0 24 24"><path d="M5 3l14 9-14 9z"/></svg>START</button>
    <button class="btn btn-x" onclick="doStop()"><svg viewBox="0 0 24 24"><rect x="5" y="5" width="14" height="14" rx="2"/></svg>STOP</button>
  </div>
</div>

<div id="pnl-sensors">
  <div class="statgrid">
    <div class="stat" id="mVib">
      <div class="stat-top"><div class="stat-ico cyan"><svg viewBox="0 0 24 24"><path d="M2 12c2 0 2-5 4-5s2 10 4 10 2-10 4-10 2 5 4 5"/></svg></div></div>
      <div class="stat-lbl">Getaran</div>
      <div class="stat-val"><span id="vib">—</span><span class="stat-unit">m/s²</span></div>
      <div class="stat-sub" id="vib_base">baseline gravitasi: <b>— m/s²</b></div>
    </div>
    <div class="stat" id="mCur">
      <div class="stat-top"><div class="stat-ico pur"><svg viewBox="0 0 24 24"><path d="M13 2L3 14h8l-1 8 10-12h-8z"/></svg></div></div>
      <div class="stat-lbl">Arus motor</div>
      <div class="stat-val"><span id="cur">—</span><span class="stat-unit">A</span></div>
      <div class="stat-sub">sensor ACS712</div>
    </div>
    <div class="stat" id="mPrs">
      <div class="stat-top"><div class="stat-ico grn"><svg viewBox="0 0 24 24"><path d="M12 14a8 8 0 1 0-8-8"/><path d="M12 14l4-4"/><circle cx="12" cy="14" r="1.4"/></svg></div></div>
      <div class="stat-lbl">Tekanan suction</div>
      <div class="stat-val"><span id="prs_kpa">—</span><span class="stat-unit">kPa</span></div>
      <div class="stat-sub"><span id="prs_bar">≈ — Bar</span> · <span id="prs_pct">—</span></div>
    </div>
    <div class="stat">
      <div class="stat-top"><div class="stat-ico amb"><svg viewBox="0 0 24 24"><path d="M22 12h-4l-3 9L9 3l-3 9H2"/></svg></div></div>
      <div class="stat-lbl">PWM aktual</div>
      <div class="stat-val"><span id="pwm">—</span><span class="stat-unit">%</span></div>
      <div class="stat-sub">duty siklus pompa</div>
    </div>
  </div>
</div>

<div class="row2">
  <div id="pnl-cav">
    <div class="cav n" id="cavCard" style="height:100%">
      <div class="cav-lbl">Indeks Kavitasi · Fuzzy Mamdani</div>
      <div class="ring-wrap">
        <svg width="138" height="138" viewBox="0 0 138 138">
          <circle cx="69" cy="69" r="52" fill="none" stroke="#1a2740" stroke-width="11"/>
          <circle id="cavRing" cx="69" cy="69" r="52" fill="none" stroke="#10b981" stroke-width="11"
            stroke-linecap="round" stroke-dasharray="326.726" stroke-dashoffset="326.726"
            style="transition:stroke-dashoffset .5s,stroke .4s"/>
        </svg>
        <div class="ring-c"><div class="cav-val" id="cavVal">0.00</div><small>0 – 1.5</small></div>
      </div>
      <span class="cav-st n" id="cavSt">NORMAL</span>
    </div>
  </div>
  <div id="pnl-pid">
    <div class="pid-wrap" style="height:100%">
      <div class="pid-top">
        <span class="lbl">PID Controller<span class="pid-mode-tag V" id="pidModeTag">VIBRASI</span></span>
        <span class="pid-sp" id="pidSpInfo">SP: — · PV: —</span>
      </div>
      <div class="pid-row"><span class="pid-lbl">PID output</span><div class="pid-track"><div class="pid-fill" id="pidFill" style="width:0%"></div></div><span class="pid-num" id="pidVal">0.0000</span></div>
      <div class="pid-row"><span class="pid-lbl">PWM target</span><div class="pid-track"><div class="pid-fill" id="pwmFill" style="width:0%;background:linear-gradient(90deg,var(--cyan2),var(--cyan))"></div></div><span class="pid-num" id="pwmTgt">0</span></div>
    </div>
  </div>
</div>

<div id="pnl-chart">
  <div class="chart-card">
    <div class="card-h"><div class="t"><div class="ic" style="background:linear-gradient(135deg,var(--cyan),var(--pri))"><svg viewBox="0 0 24 24"><path d="M3 3v18h18"/><path d="M7 14l3-4 3 3 5-7"/></svg></div>Trend Chart Real-Time</div><span class="chart-info">Window <b>40 dtk</b></span></div>
    <canvas id="trendCanvas" onmousemove="chartMouseMove(event)" onmouseleave="chartMouseLeave()"></canvas>
    <div class="chart-legend">
      <div class="chart-leg" id="legd_vib" onclick="chartToggleChannel('vib')"><span class="dot" style="background:#ef4444"></span><span class="lbl">Vibrasi<span>m/s²</span></span></div>
      <div class="chart-leg" id="legd_cur" onclick="chartToggleChannel('cur')"><span class="dot" style="background:#10b981"></span><span class="lbl">Arus<span>A</span></span></div>
      <div class="chart-leg" id="legd_prs" onclick="chartToggleChannel('prs')"><span class="dot" style="background:#22d3ee"></span><span class="lbl">Tekanan<span>kPa</span></span></div>
      <div class="chart-leg" id="legd_pwm" onclick="chartToggleChannel('pwm')"><span class="dot" style="background:#f59e0b"></span><span class="lbl">PWM<span>%</span></span></div>
      <div class="chart-leg off" id="legd_cav" onclick="chartToggleChannel('cav')"><span class="dot" style="background:#a855f7"></span><span class="lbl">CavIdx<span>0–2</span></span></div>
    </div>
    <div class="chart-btns">
      <button class="chart-btn chart-btn-png" onclick="downloadChartPNG()">Unduh Chart PNG</button>
      <button class="chart-btn chart-btn-ui" onclick="downloadFullUI()">Unduh UI Penuh</button>
      <button class="chart-btn chart-btn-clr" onclick="chartClear()">Bersihkan Buffer</button>
    </div>
  </div>
</div>
</div>

<!-- ═══ FUZZY MF ══════════════════════════════════════════════════════ -->
<div id="page-fuzzy" class="page">
<div class="pg-hdr"><div><h1>Fuzzy <b>Membership</b></h1><div class="sub">Fungsi keanggotaan &amp; bobot Mamdani 27-aturan</div></div></div>
<div id="pnl-mf">
  <div class="mf-card">
    <div class="card-h"><div class="t"><div class="ic" style="background:linear-gradient(135deg,var(--cyan),var(--cyan2))"><svg viewBox="0 0 24 24"><path d="M3 12c3-7 6-7 9 0s6 7 9 0"/></svg></div>Membership Function</div><span class="mf-weights" id="mfWeights">Bobot: …</span></div>
    <div class="mf-row" id="mfRow_V"><div class="mf-name"><b>VIBRASI</b><span>m/s²</span></div><div id="mfBox_V"></div><div class="mf-val" id="mfVal_V"><b>—</b><span>m/s²</span></div></div>
    <div class="mf-row" id="mfRow_P"><div class="mf-name"><b>TEKANAN</b><span>kPa</span></div><div id="mfBox_P"></div><div class="mf-val" id="mfVal_P"><b>—</b><span>kPa</span></div></div>
    <div class="mf-row" id="mfRow_I"><div class="mf-name"><b>ARUS</b><span>A</span></div><div id="mfBox_I"></div><div class="mf-val" id="mfVal_I"><b>—</b><span>A</span></div></div>
    <div class="mf-legend"><span class="lo">LOW</span><span class="no">NORMAL/MED</span><span class="hi">HIGH</span><span class="sp">Setpoint</span><span class="pv">Nilai aktual</span></div>
  </div>
  <div class="info-box">
    <b style="color:var(--cyan)">Sistem Fuzzy Mamdani 27 Aturan (3×3×3)</b><br>
    Input: P (tekanan suction) · V (getaran) · I (arus motor)<br>
    Output CavIdx: 0.0 = Normal · 0.5 = Kavitasi awal · 1.5 = Kavitasi parah<br>
    Deteksi butuh <b style="color:var(--amb2)">minimal 2 sensor konfirmasi</b>. Ambang tekanan proporsional terhadap kecepatan PWM.
  </div>
</div>
</div>

<!-- ═══ KONTROL ════════════════════════════════════════════════════════ -->
<div id="page-kontrol" class="page">
<div class="pg-hdr"><div><h1>Kontrol <b>Sistem</b></h1><div class="sub">Sensor, mode prioritas &amp; uji tutup-katup</div></div></div>
<div id="pnl-sen-en">
  <div class="sens-card">
    <div class="sens-title"><span>Aktif / Nonaktif Sensor</span><span class="sens-mode" id="fmode">MODE: VPI (3/3)</span></div>
    <div class="sens-row">
      <div class="sens-item on" id="si_V"><div class="sens-name on" id="sl_V">VIBRASI</div><label class="tog"><input type="checkbox" id="s_V" checked onchange="toggleSensor('V')"><span class="tog-slider"></span></label><div class="sens-status" id="ss_V">AKTIF</div></div>
      <div class="sens-item on" id="si_P"><div class="sens-name on" id="sl_P">TEKANAN</div><label class="tog"><input type="checkbox" id="s_P" checked onchange="toggleSensor('P')"><span class="tog-slider"></span></label><div class="sens-status" id="ss_P">AKTIF</div></div>
      <div class="sens-item on" id="si_I"><div class="sens-name on" id="sl_I">ARUS</div><label class="tog"><input type="checkbox" id="s_I" checked onchange="toggleSensor('I')"><span class="tog-slider"></span></label><div class="sens-status" id="ss_I">AKTIF</div></div>
    </div>
  </div>
</div>
<div id="pnl-prior">
  <div class="sens-card" style="border-color:rgba(34,211,238,.4)">
    <div class="sens-title" style="color:var(--cyan)"><span>Prioritas Sistem (Program)</span><span class="sens-mode" id="ctrlNow">PID: ARUS</span></div>
    <div style="display:grid;grid-template-columns:1fr 1fr;gap:9px;margin-bottom:11px">
      <div class="mini">
        <div class="mini-lbl">Kontrol PID</div>
        <div class="mono" style="font-size:.74rem;margin-top:4px"><span style="color:var(--pur);font-weight:700">I</span><span style="color:var(--dim)"> → </span><span style="color:var(--grn2);font-weight:700">V</span><span style="color:var(--dim)"> → </span><span style="color:var(--cyan);font-weight:700">P</span></div>
        <div class="mini-sub">Arus → Vibrasi → Tekanan</div>
      </div>
      <div class="mini">
        <div class="mini-lbl">Bobot Fuzzy</div>
        <div class="mono" style="font-size:.74rem;margin-top:4px"><span style="color:var(--cyan);font-weight:700">P</span><span style="color:var(--dim)">=0.40 · </span><span style="color:var(--grn2);font-weight:700">V</span><span style="color:var(--dim)">=0.40 · </span><span style="color:var(--pur);font-weight:700">I</span><span style="color:var(--dim)">=0.20</span></div>
        <div class="mini-sub">P = V &gt; I</div>
      </div>
    </div>
    <div class="help mono">PID aktif: <b id="pidActLbl" style="color:var(--cyan)">—</b> · Prioritas ditetapkan program (config.h), tidak dapat diubah via UI.</div>
  </div>
</div>
<div id="pnl-isp">
  <div class="sens-card" style="border-color:rgba(168,85,247,.4)">
    <div class="sens-title" style="color:var(--pur)"><span>Target Kecepatan — Setpoint Arus</span><span class="sens-mode" id="ispNow" style="color:var(--pur);background:rgba(168,85,247,.13)">50%</span></div>
    <div style="display:grid;grid-template-columns:1fr 1fr 1fr;gap:9px;margin-bottom:11px">
      <div class="mini"><div class="mini-lbl">Target nominal</div><div class="mini-val" style="color:var(--pur)"><span id="ispval">50</span><span style="font-size:.62rem;color:var(--dim)">%</span></div></div>
      <div class="mini"><div class="mini-lbl">SP arus nominal</div><div class="mini-val" style="color:var(--pur)" id="ispSpA">— A</div></div>
      <div class="mini"><div class="mini-lbl">Efektif (kavitasi)</div><div class="mini-val"><span id="ispEffNow" style="color:var(--grn2)">50%</span></div></div>
    </div>
    <div class="mini mono" id="ispRedBox" style="font-size:.6rem;margin-bottom:11px;color:var(--grn2)"><span id="ispReduction">Normal (100%) — tidak ada reduksi</span></div>
    <div class="sld-row"><input type="range" id="ispsl" min="10" max="100" value="50" oninput="setIspLive(this.value)"></div>
    <div class="help">Hanya berlaku pada <b style="color:var(--pur)">Mode Kontrol ARUS</b>. SP efektif turun otomatis saat kavitasi: Early=80%, Severe=60% dari nominal.<br>SP_A = slope × pct + offset. Geser slider → diterapkan otomatis setelah 0.6 detik.</div>
  </div>
</div>
<div id="pnl-vtest">
  <div class="sens-card" style="border-color:rgba(245,158,11,.4)">
    <div class="sens-title" style="color:var(--amb2)"><span>Uji Tutup-Katup (PWM Tetap)</span><span class="sens-mode" id="vtNow" style="color:var(--dim);background:var(--inset)">NONAKTIF</span></div>
    <div class="btns">
      <button class="btn" onclick="setVtest(1)" style="background:linear-gradient(135deg,var(--amb),var(--ora))">AKTIFKAN</button>
      <button class="btn" onclick="setVtest(0)" style="background:var(--inset);color:var(--amb2);border:1px solid rgba(245,158,11,.4)">NONAKTIF</button>
    </div>
    <div class="sld-row"><input type="range" id="vtsl" min="0" max="100" value="50" style="accent-color:var(--amb)" oninput="setVtestPwm(this.value)"><span class="sld-v" id="vtval" style="color:var(--amb2)">50</span><span style="font-size:.66rem;color:var(--dim)">%</span></div>
    <div class="help">PWM dikunci tetap; PID &amp; severe-override MATI.<br>START → AKTIFKAN → atur PWM → tutup katup bertahap → rekam CSV tiap posisi.</div>
  </div>
</div>
</div>

<!-- ═══ KALIBRASI ═════════════════════════════════════════════════════ -->
<div id="page-kalibrasi" class="page">
<div class="pg-hdr"><div><h1>Kalibrasi <b>Sensor</b></h1><div class="sub">ACS712 · MPU6050 · WPT-83G</div></div></div>
<div id="pnl-acal">
  <div class="card" style="border-color:rgba(168,85,247,.4)">
    <div class="card-h"><div class="t"><div class="ic" style="background:linear-gradient(135deg,var(--pur),var(--pri))"><svg viewBox="0 0 24 24"><path d="M13 2L3 14h8l-1 8 10-12h-8z"/></svg></div>Sensor Arus ACS712</div><span class="badge def" id="acalBadge">DEFAULT</span></div>
    <div class="xinfo">
      <div class="xinfo-item"><div class="xinfo-lbl">Zero Offset (V)</div><div class="xinfo-val pur" id="acalOffset">2.50000 V</div></div>
      <div class="xinfo-item"><div class="xinfo-lbl">Gain koreksi</div><div class="xinfo-val pur" id="acalGain">1.00000</div></div>
      <div class="xinfo-item" style="grid-column:1/-1"><div class="xinfo-lbl">Arus terukur sekarang</div><div class="xinfo-val pur" id="acalINow">— A</div></div>
    </div>
    <div class="xbtns">
      <button class="xbtn xbtn-pri" onclick="acalZero()">① Set Zero</button>
      <button class="xbtn xbtn-rst" onclick="acalReset()">↺ Reset</button>
    </div>
    <div class="xspan-row"><span>I_ref:</span><input type="number" id="acalRef" class="pur" placeholder="A dari clamp meter" step="0.001" min="0.05"><span>A</span><button class="xbtn xbtn-pri2" style="flex:0 0 auto;padding:9px 14px" onclick="acalSpan()">② Set Span</button></div>
    <div class="xprog" id="acalProg"><div class="xprog-fill" id="acalProgFill" style="width:0%"></div></div>
    <div class="xhelp pur"><b class="pur">Kalibrasi 2-langkah:</b>① ZERO: pompa OFF, tidak ada arus → [Set Zero]<br>② SPAN (opsional): pompa ON stabil, baca clamp meter → masukkan nilai → [Set Span]<br>Tersimpan di flash ESP32 (NVS). Gain valid: 0.3–3.0×</div>
    <div class="xmsg" id="acalMsg"></div>
  </div>
</div>
<div id="pnl-mcal">
  <div class="card" style="border-color:rgba(16,185,129,.4)">
    <div class="card-h"><div class="t"><div class="ic" style="background:linear-gradient(135deg,var(--grn2),var(--grn))"><svg viewBox="0 0 24 24"><path d="M2 12c2 0 2-5 4-5s2 10 4 10 2-10 4-10 2 5 4 5"/></svg></div>Sensor Vibrasi MPU6050</div><span class="badge def" id="mcalBadge">DEFAULT</span></div>
    <div class="xinfo">
      <div class="xinfo-item"><div class="xinfo-lbl">Offset Ax (m/s²)</div><div class="xinfo-val grn" id="mcalAx">0.00000</div></div>
      <div class="xinfo-item"><div class="xinfo-lbl">Offset Ay (m/s²)</div><div class="xinfo-val grn" id="mcalAy">0.00000</div></div>
      <div class="xinfo-item"><div class="xinfo-lbl">Offset Az (m/s²)</div><div class="xinfo-val grn" id="mcalAz">0.00000</div></div>
      <div class="xinfo-item"><div class="xinfo-lbl">Vibrasi sekarang</div><div class="xinfo-val grn" id="mcalVibNow">—</div></div>
    </div>
    <div class="xbtns">
      <button class="xbtn xbtn-grn" onclick="mcalStart()">Kalibrasi Ulang</button>
      <button class="xbtn xbtn-rst" onclick="mcalReset()">↺ Reset</button>
    </div>
    <div class="xprog" id="mcalProg"><div class="xprog-fill" id="mcalProgFill" style="width:0%"></div></div>
    <div class="xhelp grn"><b class="grn">Syarat kalibrasi MPU6050:</b>• Pompa harus MATI (tidak ada getaran motor)<br>• Sensor sudah terpasang di posisi final<br>• Jangan goyang sistem selama ~2 detik proses<br>Tersimpan di flash ESP32 (NVS).</div>
    <div class="xmsg" id="mcalMsg"></div>
  </div>
</div>
<div id="pnl-pcal">
  <div class="card" style="border-color:rgba(245,158,11,.4)">
    <div class="card-h"><div class="t"><div class="ic" style="background:linear-gradient(135deg,var(--amb),var(--ora))"><svg viewBox="0 0 24 24"><path d="M12 14a8 8 0 1 0-8-8"/><path d="M12 14l4-4"/></svg></div>Sensor Tekanan WPT-83G</div><span class="badge def" id="pcalBadge">DEFAULT</span></div>
    <div class="xinfo">
      <div class="xinfo-item"><div class="xinfo-lbl">Zero offset</div><div class="xinfo-val amb" id="pcalZero">0.00 kPa</div></div>
      <div class="xinfo-item"><div class="xinfo-lbl">Gain correction</div><div class="xinfo-val amb" id="pcalGain">1.0000</div></div>
      <div class="xinfo-item" style="grid-column:1/-1"><div class="xinfo-lbl">Metode</div><div class="xinfo-val amb" id="pcalMethod" style="font-size:.74rem">DEFAULT</div></div>
    </div>
    <div class="xbtns">
      <button class="xbtn xbtn-grn" onclick="pcalZero()">① Set Zero</button>
      <button class="xbtn xbtn-rst" onclick="pcalReset()">↺ Reset</button>
    </div>
    <div class="xspan-row"><span>P_ref:</span><input type="number" id="pcalRef" class="amb" placeholder="kPa dari manometer" step="0.1" min="1"><span>kPa</span><button class="xbtn xbtn-amb" style="flex:0 0 auto;padding:9px 14px" onclick="pcalSpan()">② Set Span</button></div>
    <div class="xhelp amb"><b class="amb">Kalibrasi 2-langkah:</b>① ZERO: pompa OFF, terbuka ke udara → [Set Zero]<br>② SPAN (opsional): pompa ON, baca manometer → masukkan nilai → [Set Span]<br>Tersimpan permanen di flash ESP32 (NVS).</div>
    <div class="xmsg" id="pcalMsg"></div>
  </div>
</div>
<div id="pnl-spinfo">
  <div class="info-box">
    SP vibrasi: <b id="warnSpV" style="color:var(--red)">—</b> · SP tekanan: <b id="warnSpP" style="color:var(--cyan)">—</b> · SP arus: adaptif (slope×target+offset) · Kalibrasi sensor sebelum operasi.
  </div>
</div>
</div>

<!-- ═══ LOGGER ════════════════════════════════════════════════════════ -->
<div id="page-logger" class="page">
<div class="pg-hdr"><div><h1>Data <b>Logger</b></h1><div class="sub">Perekaman CSV di RAM ESP32</div></div></div>
<div id="pnl-log">
  <div class="card" style="border-color:rgba(168,85,247,.4)">
    <div class="card-h"><div class="t"><span class="log-dot" id="logDot"></span>Data Logger CSV</div></div>
    <div class="log-int-row"><span>Interval:</span><input type="number" id="logIntervalInput" value="500" min="100" max="60000" step="100"><span>ms</span></div>
    <div class="log-btns">
      <button class="lbtn lb-start" id="btnLogStart" onclick="logStart()">Start</button>
      <button class="lbtn lb-stop" id="btnLogStop" onclick="logStop()" disabled>Stop</button>
      <button class="lbtn lb-dl" id="btnLogDl" onclick="downloadCSV()" disabled>CSV</button>
      <button class="lbtn lb-clear" onclick="logClear()">Clear</button>
    </div>
    <div class="log-stats">
      <div class="log-stat"><div class="log-stat-lbl">Baris</div><div class="log-stat-val" id="logCountVal">0</div></div>
      <div class="log-stat"><div class="log-stat-lbl">RAM</div><div class="log-stat-val" id="logSizeVal">0 KB</div></div>
      <div class="log-stat"><div class="log-stat-lbl">Heap</div><div class="log-stat-val" id="logHeapVal">—</div></div>
      <div class="log-stat"><div class="log-stat-lbl">Interval</div><div class="log-stat-val" id="logIntVal">—</div></div>
    </div>
    <div class="prog-cap"><span>Buffer</span><span id="logProgPct">0%</span></div>
    <div class="log-prog-track"><div class="log-prog-fill" id="logProgFill" style="width:0%"></div></div>
  </div>
  <div class="info-box">
    <b style="color:var(--pur)">Format CSV:</b> Time_ms · Vibration_m/s² · Current_A · Pressure_kPa · Pressure_bar · PWM_% · CavIdx · CavState · PIDout · SensorStatus · FuzzyMode · PID_Mode · EnableMask
  </div>
</div>
</div>

<!-- ═══ SISTEM ════════════════════════════════════════════════════════ -->
<div id="page-sistem" class="page">
<div class="pg-hdr"><div><h1>Sistem <b>ESP32-S3</b></h1><div class="sub">Monitor internal mikrokontroler</div></div></div>
<div id="pnl-sys">
  <div class="tm-card" id="tmCard">
    <div class="card-h"><div class="t"><div class="ic" style="background:linear-gradient(135deg,var(--pri),var(--pri2))"><svg viewBox="0 0 24 24"><rect x="4" y="4" width="16" height="16" rx="2"/><rect x="9" y="9" width="6" height="6"/><path d="M9 1v3M15 1v3M9 20v3M15 20v3M1 9h3M1 15h3M20 9h3M20 15h3"/></svg></div>System Monitor</div><span class="badge ok" id="tmBadge">NORMAL</span></div>
    <div class="tm-grid">
      <div class="tm-item"><div class="tm-item-lbl">Chip temp</div><div class="tm-item-val" id="tmTemp" style="color:var(--grn2)">—</div><div class="tm-item-sub">throttle ≥ 80°C</div></div>
      <div class="tm-item"><div class="tm-item-lbl">CPU freq</div><div class="tm-item-val" id="tmCpu" style="color:var(--cyan)">— MHz</div><div class="tm-item-sub" id="tmThr" style="color:var(--grn2)">Normal</div></div>
      <div class="tm-item"><div class="tm-item-lbl">Loop / s</div><div class="tm-item-val" id="tmLps" style="color:var(--pur)">—</div><div class="tm-item-sub">throughput</div></div>
      <div class="tm-item"><div class="tm-item-lbl">Bandwidth</div><div class="tm-item-val" id="tmMbps" style="color:var(--amb2)">— KB/s</div><div class="tm-item-sub">WiFi /data</div></div>
      <div class="tm-item"><div class="tm-item-lbl">Free heap</div><div class="tm-item-val" id="tmHeap" style="color:var(--cyan)">— KB</div><div class="tm-item-sub">RAM tersedia</div></div>
      <div class="tm-item"><div class="tm-item-lbl">Uptime</div><div class="tm-item-val" id="tmUp" style="color:var(--grn2)">—</div><div class="tm-item-sub">sejak boot</div></div>
    </div>
  </div>
  <div class="info-box">
    <b style="color:var(--cyan)">SmartPump</b> · ESP32-S3 · Fuzzy Mamdani 27-Rule · FGS PID · Speed-Proportional Cavitation Detection<br>
    Ambang kavitasi: P_early=0.40 kPa · P_severe=0.10 kPa (pada PWM_ref=100) · Konfirmasi 800ms.
  </div>
</div>
</div>

<!-- ═══ MODAL POPUP ═══════════════════════════════════════════════════ -->
<div class="modal-overlay" id="modalOverlay">
  <div class="modal">
    <div class="modal-ico warn" id="modalIco"></div>
    <div class="modal-title" id="modalTitle">Konfirmasi</div>
    <div class="modal-msg" id="modalMsg"></div>
    <div class="modal-btns">
      <button class="mbtn mbtn-cancel" id="modalCancel" onclick="_modalDone(false)">Batal</button>
      <button class="mbtn mbtn-confirm" id="modalConfirm" onclick="_modalDone(true)">Lanjut</button>
    </div>
  </div>
</div>
<div id="toastWrap"></div>

<script>
// ════════ Ikon inline ════════
var ICO={
  warn:'<svg viewBox="0 0 24 24"><path d="M12 2L1 21h22z"/><path d="M12 9v5"/><path d="M12 17.5h.01"/></svg>',
  danger:'<svg viewBox="0 0 24 24"><path d="M3 6h18M8 6V4h8v2M6 6l1 14h10l1-14"/><path d="M10 11v6M14 11v6"/></svg>',
  ok:'<svg viewBox="0 0 24 24"><path d="M20 6L9 17l-5-5"/></svg>',
  info:'<svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="10"/><path d="M12 16v-5"/><path d="M12 8h.01"/></svg>',
  bell:'<svg viewBox="0 0 24 24"><path d="M18 8a6 6 0 0 0-12 0c0 7-3 9-3 9h18s-3-2-3-9"/></svg>'
};

// ════════ Modal popup ════════
var _modalResolve=null;
function showModal(opt){
  return new Promise(function(res){
    _modalResolve=res;var o=opt||{};
    var ico=document.getElementById('modalIco');
    ico.className='modal-ico '+(o.type||'warn');ico.innerHTML=o.iconSvg||ICO.warn;
    document.getElementById('modalTitle').innerText=o.title||'Konfirmasi';
    document.getElementById('modalMsg').innerHTML=o.msg||'';
    var cancel=document.getElementById('modalCancel');
    cancel.style.display=o.alert?'none':'';cancel.innerText=o.cancelText||'Batal';
    var conf=document.getElementById('modalConfirm');
    conf.innerText=o.confirmText||'Lanjut';
    conf.className='mbtn '+(o.danger?'mbtn-danger':'mbtn-confirm');
    document.getElementById('modalOverlay').classList.add('open');
  });
}
function _modalDone(v){document.getElementById('modalOverlay').classList.remove('open');if(_modalResolve){var r=_modalResolve;_modalResolve=null;r(v);}}
function showAlert(msg,title,type){return showModal({alert:true,msg:msg,title:title||'Perhatian',type:type||'info',iconSvg:ICO.info,confirmText:'Mengerti'});}
document.getElementById('modalOverlay').addEventListener('click',function(e){if(e.target===this)_modalDone(false);});

// ════════ Toast ════════
function showToast(msg,type){
  var w=document.getElementById('toastWrap');var t=document.createElement('div');
  t.className='toast '+(type||'');t.innerText=msg;w.appendChild(t);
  setTimeout(function(){t.classList.add('hide');setTimeout(function(){if(t.parentNode)t.parentNode.removeChild(t);},260);},3500);
}

// ════════ Notifikasi ════════
var NOTIFS=[],NID=0;
function relTime(ms){var s=Math.floor((Date.now()-ms)/1000);if(s<5)return'baru saja';if(s<60)return s+' dtk lalu';var m=Math.floor(s/60);if(m<60)return m+' mnt lalu';var h=Math.floor(m/60);return h+' jam lalu';}
function pushNotif(title,msg,sev){
  NOTIFS.unshift({id:++NID,title:title,msg:msg,sev:sev||'info',t:Date.now(),read:false});
  if(NOTIFS.length>30)NOTIFS.pop();
  renderNotif();
  if(sev==='crit'||sev==='warn')showToast(title,sev==='crit'?'err':'warn');
}
function renderNotif(){
  var list=document.getElementById('notifList'),empty=document.getElementById('notifEmpty');
  var unread=NOTIFS.filter(function(n){return!n.read;}).length;
  var badge=document.getElementById('notifBadge');
  badge.innerText=unread>9?'9+':unread;badge.classList.toggle('show',unread>0);
  if(NOTIFS.length===0){list.innerHTML='';empty.style.display='block';return;}
  empty.style.display='none';
  var icoMap={crit:ICO.warn,warn:ICO.warn,ok:ICO.ok,info:ICO.info};
  list.innerHTML=NOTIFS.map(function(n){
    return '<div class="notif-item'+(n.read?'':' unread')+'"><div class="ni-ico '+n.sev+'">'+(icoMap[n.sev]||ICO.info)+'</div>'
      +'<div class="ni-body"><div class="ni-title">'+n.title+'</div><div class="ni-msg">'+n.msg+'</div><div class="ni-time">'+relTime(n.t)+'</div></div></div>';
  }).join('');
}
function toggleNotif(){
  var p=document.getElementById('notifPanel'),b=document.getElementById('notifBtn');
  var open=p.classList.toggle('open');b.classList.toggle('on',open);
  closeVis();
  if(open){NOTIFS.forEach(function(n){n.read=true;});renderNotif();}
}
function markAllRead(){NOTIFS.forEach(function(n){n.read=true;});renderNotif();}
function closeNotif(){document.getElementById('notifPanel').classList.remove('open');document.getElementById('notifBtn').classList.remove('on');}
function closeVis(){document.getElementById('visPanel').classList.remove('open');document.getElementById('navVisBtn').classList.remove('on');}
setInterval(function(){if(document.getElementById('notifPanel').classList.contains('open'))renderNotif();},10000);

// State-change → notifikasi
var NT={started:false,cav:0,thr:false,stop:null,lastCrit:0};
function checkNotif(d){
  if(!NT.started){NT.cav=d.cs;NT.thr=d.thr;NT.stop=d.stop;NT.started=true;return;}
  if(d.cs!==NT.cav){
    if(d.cs===2)pushNotif('Kavitasi parah terdeteksi','PWM diturunkan otomatis untuk melindungi pompa.','crit');
    else if(d.cs===1)pushNotif('Kavitasi awal terdeteksi','Indeks kavitasi melewati ambang dini.','warn');
    else if(d.cs===0&&NT.cav>0)pushNotif('Kavitasi pulih','Sistem kembali ke kondisi normal.','ok');
    NT.cav=d.cs;
  }
  if(d.thr&&!NT.thr)pushNotif('Throttle termal aktif','Suhu chip tinggi, frekuensi CPU diturunkan.','warn');
  if(!d.thr&&NT.thr)pushNotif('Suhu chip normal','Throttle termal nonaktif.','ok');
  NT.thr=d.thr;
  if(d.temp>=90&&NT.lastCrit+30000<Date.now()){pushNotif('Suhu chip kritis','Chip ESP32-S3 melewati 90°C.','crit');NT.lastCrit=Date.now();}
  if(NT.stop!==null&&d.stop!==NT.stop){if(d.stop)pushNotif('Pompa dihentikan','Perintah STOP diterima.','info');else pushNotif('Pompa dijalankan','Sistem mulai beroperasi.','ok');}
  NT.stop=d.stop;
}

// ════════ Page & Visibility panel ════════
var PAGES=['beranda','fuzzy','kontrol','kalibrasi','logger','sistem'];
var PAGE_PANELS={
  beranda:[{id:'pnl-ctrl',label:'Kontrol Pompa'},{id:'pnl-sensors',label:'Stat Cards'},{id:'pnl-cav',label:'Ring Kavitasi'},{id:'pnl-pid',label:'PID Controller'},{id:'pnl-chart',label:'Trend Chart'}],
  fuzzy:[{id:'pnl-mf',label:'Membership Function'}],
  kontrol:[{id:'pnl-sen-en',label:'Aktif Sensor'},{id:'pnl-prior',label:'Prioritas Kontrol'},{id:'pnl-isp',label:'Setpoint Arus'},{id:'pnl-vtest',label:'Uji Tutup-Katup'}],
  kalibrasi:[{id:'pnl-acal',label:'Kalibrasi ACS712'},{id:'pnl-mcal',label:'Kalibrasi MPU6050'},{id:'pnl-pcal',label:'Kalibrasi Tekanan'},{id:'pnl-spinfo',label:'Info Setpoint'}],
  logger:[{id:'pnl-log',label:'Data Logger CSV'}],
  sistem:[{id:'pnl-sys',label:'System Monitor'}]
};
var BERANDA_EXTRAS=[
  {id:'pnl-mf',label:'Fuzzy MF',src:'page-fuzzy'},
  {id:'pnl-sen-en',label:'Aktif Sensor',src:'page-kontrol'},
  {id:'pnl-prior',label:'Prioritas Kontrol',src:'page-kontrol'},
  {id:'pnl-isp',label:'Setpoint Arus',src:'page-kontrol'},
  {id:'pnl-vtest',label:'Uji Tutup-Katup',src:'page-kontrol'},
  {id:'pnl-acal',label:'Kalibrasi ACS712',src:'page-kalibrasi'},
  {id:'pnl-mcal',label:'Kalibrasi MPU6050',src:'page-kalibrasi'},
  {id:'pnl-pcal',label:'Kalibrasi Tekanan',src:'page-kalibrasi'},
  {id:'pnl-spinfo',label:'Info Setpoint',src:'page-kalibrasi'},
  {id:'pnl-log',label:'Data Logger CSV',src:'page-logger'},
  {id:'pnl-sys',label:'System Monitor',src:'page-sistem'},
];
var curPage='beranda';
function showPage(p){
  curPage=p;
  PAGES.forEach(function(id){
    var pg=document.getElementById('page-'+id);var tb=document.querySelector('[data-page="'+id+'"]');
    if(pg)pg.classList.toggle('active',id===p);if(tb)tb.classList.toggle('active',id===p);
  });
  renderVisPanel(p);closeVis();closeNotif();
  if(p==='beranda')setTimeout(drawChart,60);
  if(p==='fuzzy'&&MF_DATA)setTimeout(function(){renderMF('V',null);renderMF('P',null);renderMF('I',null);},60);
}
function toggleVisPanel(){
  var vp=document.getElementById('visPanel'),vb=document.getElementById('navVisBtn');
  var open=vp.classList.toggle('open');vb.classList.toggle('on',open);closeNotif();
  if(open)renderVisPanel(curPage);
}
document.addEventListener('click',function(e){
  if(!e.target.closest('#visPanel')&&!e.target.closest('#navVisBtn'))closeVis();
  if(!e.target.closest('#notifPanel')&&!e.target.closest('#notifBtn'))closeNotif();
});
function renderVisPanel(page){
  var panels=PAGE_PANELS[page]||[];var html='';
  panels.forEach(function(p){
    var vis=localStorage.getItem('vis-'+p.id)!=='0';
    html+='<div class="vis-item"><input type="checkbox" id="vc-'+p.id+'"'+(vis?' checked':'')+' onchange="togglePanel(\''+p.id+'\',this.checked)"><label for="vc-'+p.id+'">'+p.label+'</label></div>';
  });
  if(page==='beranda'){
    html+='<div class="vis-sep">Tampilkan di Beranda</div>';
    BERANDA_EXTRAS.forEach(function(p){
      var onHome=localStorage.getItem('home-'+p.id)==='1';
      html+='<div class="vis-item"><input type="checkbox" id="vc-h-'+p.id+'"'+(onHome?' checked':'')+' onchange="pinToHome(\''+p.id+'\',this.checked)"><label for="vc-h-'+p.id+'">'+p.label+'</label></div>';
    });
  }
  document.getElementById('visPanelItems').innerHTML=html;
  panels.forEach(function(p){var el=document.getElementById(p.id);if(el&&el.closest&&el.closest('#page-'+page))el.style.display=localStorage.getItem('vis-'+p.id)==='0'?'none':'';});
}
function togglePanel(id,vis){localStorage.setItem('vis-'+id,vis?'1':'0');var el=document.getElementById(id);if(el)el.style.display=vis?'':'none';}
function showAllPanels(){var panels=PAGE_PANELS[curPage]||[];panels.forEach(function(p){localStorage.removeItem('vis-'+p.id);var el=document.getElementById(p.id);if(el)el.style.display='';});renderVisPanel(curPage);}
function pinToHome(id,en){
  var p=BERANDA_EXTRAS.filter(function(x){return x.id===id;})[0];if(!p)return;
  var el=document.getElementById(p.id);if(!el)return;
  var beranda=document.getElementById('page-beranda');
  if(en){
    beranda.appendChild(el);el.style.display='';localStorage.setItem('home-'+id,'1');
  }else{
    var srcPage=document.getElementById(p.src);
    var tabName=p.src.replace('page-','');
    var tabPanels=PAGE_PANELS[tabName]||[];
    var idx=-1;for(var i=0;i<tabPanels.length;i++)if(tabPanels[i].id===id){idx=i;break;}
    var inserted=false;
    for(var j=idx+1;j<tabPanels.length;j++){
      var next=document.getElementById(tabPanels[j].id);
      if(next&&next.parentElement&&next.parentElement.id===p.src){srcPage.insertBefore(el,next);inserted=true;break;}
    }
    if(!inserted)srcPage.appendChild(el);
    el.style.display=localStorage.getItem('vis-'+id)==='0'?'none':'';
    localStorage.removeItem('home-'+id);
  }
  renderVisPanel('beranda');
}
function applyBerandaExtras(){
  var beranda=document.getElementById('page-beranda');
  BERANDA_EXTRAS.forEach(function(p){
    if(localStorage.getItem('home-'+p.id)==='1'){
      var el=document.getElementById(p.id);
      if(el&&(!el.closest||!el.closest('#page-beranda'))){beranda.appendChild(el);el.style.display='';}
    }
  });
}

// ════════ Kontrol pompa & cavitation ring ════════
function doStart(){fetch('/start').catch(function(){});}
function doStop(){fetch('/stop').catch(function(){});}
var CL=['n','e','s'],CC=['#10b981','#f59e0b','#f43f5e'],CN=['NORMAL','KAVITASI AWAL','KAVITASI PARAH'];
var RING_C=326.726;
function updateCavUI(cav,cs){
  var cls=CL[cs]||'n';
  document.getElementById('cavCard').className='cav '+cls;
  var st=document.getElementById('cavSt');st.className='cav-st '+cls;st.innerText=CN[cs]||'NORMAL';
  document.getElementById('cavVal').innerText=cav.toFixed(2);
  var ring=document.getElementById('cavRing');
  if(ring){var frac=Math.max(0,Math.min(cav/1.5,1));ring.style.strokeDashoffset=(RING_C*(1-frac)).toFixed(1);ring.style.stroke=CC[cs]||'#10b981';}
}
async function toggleSensor(id){var cb=document.getElementById('s_'+id);try{await fetch('/sensor?id='+id+'&en='+(cb.checked?1:0));}catch(e){}refreshSensorUI();}
function applySensorState(id,on){
  var item=document.getElementById('si_'+id),label=document.getElementById('sl_'+id),stat=document.getElementById('ss_'+id),cb=document.getElementById('s_'+id);
  if(cb)cb.checked=on;
  if(item)item.className='sens-item '+(on?'on':'off');
  if(label)label.className='sens-name '+(on?'on':'off');
  if(stat){stat.innerText=on?'AKTIF':'NONAKTIF';stat.style.color=on?'var(--grn2)':'var(--red)';}
  var mMap={P:'mPrs',V:'mVib',I:'mCur'};var card=document.getElementById(mMap[id]);
  if(card)card.classList.toggle('disabled',!on);
}
async function refreshSensorUI(){
  try{var s=await(await fetch('/sstatus')).json();
    applySensorState('P',s.P);applySensorState('V',s.V);applySensorState('I',s.I);
    var fm=document.getElementById('fmode');if(fm)fm.innerText='MODE: '+s.mode+' ('+s.nActive+'/3)';
  }catch(e){}
}

// ════════ Chart ════════
var CHART_BUF=200;
var CHANNELS={
  vib:{label:'Vibrasi',unit:'m/s²',color:'#ef4444',data:[],on:true,scale:'auto',min:0,max:5},
  cur:{label:'Arus',unit:'A',color:'#10b981',data:[],on:true,scale:'auto',min:0,max:1.5},
  prs:{label:'Tekanan',unit:'kPa',color:'#22d3ee',data:[],on:true,scale:'auto',min:0,max:30},
  pwm:{label:'PWM',unit:'%',color:'#f59e0b',data:[],on:true,scale:'fix',min:0,max:100},
  cav:{label:'CavIdx',unit:'',color:'#a855f7',data:[],on:false,scale:'fix',min:0,max:2}
};
var chartHoverX=-1;
function chartPush(key,val){var ch=CHANNELS[key];ch.data.push(val);if(ch.data.length>CHART_BUF)ch.data.shift();}
function chartClear(){for(var k in CHANNELS)CHANNELS[k].data=[];drawChart();}
function chartScale(ch){
  if(ch.scale==='fix')return[ch.min,ch.max];
  if(ch.data.length===0)return[ch.min,ch.max];
  var mn=Math.min.apply(null,ch.data),mx=Math.max.apply(null,ch.data);
  if(mx-mn<0.1)mx=mn+0.5;var pad=(mx-mn)*0.1;return[Math.max(0,mn-pad),mx+pad];
}
function drawChart(){
  var c=document.getElementById('trendCanvas');if(!c)return;
  var dpr=window.devicePixelRatio||1;var W=c.offsetWidth,H=c.offsetHeight;if(W<10||H<10)return;
  c.width=W*dpr;c.height=H*dpr;var ctx=c.getContext('2d');ctx.scale(dpr,dpr);
  ctx.fillStyle='#0a1322';ctx.fillRect(0,0,W,H);
  var pad={l:34,r:42,t:8,b:18};var plotW=W-pad.l-pad.r,plotH=H-pad.t-pad.b;
  ctx.strokeStyle='#1c2c47';ctx.lineWidth=0.5;ctx.font='9px monospace';ctx.fillStyle='#5a6e8c';
  for(var i=0;i<=5;i++){var y=pad.t+i*plotH/5;ctx.beginPath();ctx.moveTo(pad.l,y);ctx.lineTo(pad.l+plotW,y);ctx.stroke();ctx.fillText((5-i)*20+'%',2,y+3);}
  ctx.fillText('-40s',pad.l-3,H-4);ctx.fillText('-20s',pad.l+plotW/2-12,H-4);ctx.fillText('now',pad.l+plotW-18,H-4);
  for(var k in CHANNELS){
    var ch=CHANNELS[k];if(!ch.on||ch.data.length<2)continue;
    var sc=chartScale(ch);var mn2=sc[0],mx2=sc[1];var rng=(mx2-mn2)||1;
    ctx.strokeStyle=ch.color;ctx.lineWidth=1.7;ctx.shadowColor=ch.color;ctx.shadowBlur=5;ctx.beginPath();
    var n=ch.data.length;var startX=pad.l+plotW-(n-1)*plotW/(CHART_BUF-1);
    for(var j=0;j<n;j++){var x=startX+j*plotW/(CHART_BUF-1);var yv=pad.t+plotH-((ch.data[j]-mn2)/rng)*plotH;if(j===0)ctx.moveTo(x,yv);else ctx.lineTo(x,yv);}
    ctx.stroke();ctx.shadowBlur=0;
    var last=ch.data[n-1],yNorm=(last-mn2)/rng;
    ctx.fillStyle=ch.color;ctx.font='bold 9px monospace';
    ctx.fillText(last.toFixed(2),pad.l+plotW+2,pad.t+plotH-yNorm*plotH+3);
  }
  if(chartHoverX>=pad.l&&chartHoverX<=pad.l+plotW){
    ctx.strokeStyle='rgba(255,255,255,0.3)';ctx.lineWidth=0.5;ctx.setLineDash([3,3]);
    ctx.beginPath();ctx.moveTo(chartHoverX,pad.t);ctx.lineTo(chartHoverX,pad.t+plotH);ctx.stroke();ctx.setLineDash([]);
  }
}
function chartToggleChannel(key){CHANNELS[key].on=!CHANNELS[key].on;document.getElementById('legd_'+key).classList.toggle('off',!CHANNELS[key].on);drawChart();}
function chartMouseMove(e){var c=document.getElementById('trendCanvas');var rect=c.getBoundingClientRect();chartHoverX=e.clientX-rect.left;drawChart();}
function chartMouseLeave(){chartHoverX=-1;drawChart();}
function timestampStr(){var d=new Date();var p=function(n){return String(n).padStart(2,'0');};return d.getFullYear()+p(d.getMonth()+1)+p(d.getDate())+'_'+p(d.getHours())+p(d.getMinutes())+p(d.getSeconds());}
function downloadChartPNG(){
  var src=document.getElementById('trendCanvas');var out=document.createElement('canvas');
  var dpr=window.devicePixelRatio||1;var w=src.width,h=src.height,hH=80*dpr;
  out.width=w;out.height=h+hH;var ctx=out.getContext('2d');
  ctx.fillStyle='#070b16';ctx.fillRect(0,0,w,h+hH);
  ctx.fillStyle='#22d3ee';ctx.font='bold '+(14*dpr)+'px sans-serif';ctx.fillText('Smart Pump — Trend Chart',10*dpr,22*dpr);
  ctx.fillStyle='#cde';ctx.font=(10*dpr)+'px monospace';ctx.fillText('Diekspor: '+new Date().toLocaleString('id-ID'),10*dpr,42*dpr);
  ctx.drawImage(src,0,hH);
  var a=document.createElement('a');a.href=out.toDataURL('image/png');a.download='smart_pump_chart_'+timestampStr()+'.png';document.body.appendChild(a);a.click();document.body.removeChild(a);
}
async function downloadFullUI(){
  if(typeof html2canvas==='undefined'){
    try{await new Promise(function(res,rej){var s=document.createElement('script');s.src='https://cdn.jsdelivr.net/npm/html2canvas@1.4.1/dist/html2canvas.min.js';s.onload=res;s.onerror=rej;document.head.appendChild(s);});}
    catch(e){showToast('Gagal memuat pustaka tangkap layar (butuh internet).','err');return;}
  }
  if(typeof html2canvas==='undefined')return;
  try{var canvas=await html2canvas(document.body,{backgroundColor:'#070b16',scale:1.2});var a=document.createElement('a');a.href=canvas.toDataURL('image/png');a.download='smart_pump_ui_'+timestampStr()+'.png';document.body.appendChild(a);a.click();document.body.removeChild(a);}
  catch(e){showToast('Gagal menyimpan tangkapan: '+e.message,'err');}
}

// ════════ MF render ════════
var MF_DATA=null;
async function loadMFData(){
  try{
    MF_DATA=await(await fetch('/mfdata')).json();var w=MF_DATA.weights;
    document.getElementById('mfWeights').innerHTML='Bobot: V=<b>'+w.V.toFixed(2)+'</b> · P=<b>'+w.P.toFixed(2)+'</b> · I=<b>'+w.I.toFixed(2)+'</b>';
    renderMF('V',null);renderMF('P',null);renderMF('I',null);
  }catch(e){}
}
function renderMF(id,pv){
  if(!MF_DATA||!MF_DATA[id])return;
  var cfg=MF_DATA[id];var W=320,H=60,padL=2,padR=2,padT=4,padB=14;
  var innerW=W-padL-padR,innerH=H-padT-padB;var xMin=cfg.min,xMax=cfg.max;
  var x2px=function(v){return padL+(v-xMin)/(xMax-xMin)*innerW;};
  var y2px=function(m){return padT+(1-m)*innerH;};
  var lf=cfg.low[0],lz=cfg.low[1];var n0=cfg.norm[0],np=cfg.norm[1],nr=cfg.norm[2];var hz=cfg.high[0],hf=cfg.high[1];
  var lowPath='M'+x2px(xMin)+','+y2px(1)+' L'+x2px(lf)+','+y2px(1)+' L'+x2px(lz)+','+y2px(0)+' L'+x2px(xMin)+','+y2px(0)+' Z';
  var normPath='M'+x2px(n0)+','+y2px(0)+' L'+x2px(np)+','+y2px(1)+' L'+x2px(nr)+','+y2px(0)+' Z';
  var highPath='M'+x2px(hz)+','+y2px(0)+' L'+x2px(hf)+','+y2px(1)+' L'+x2px(xMax)+','+y2px(1)+' L'+x2px(xMax)+','+y2px(0)+' Z';
  var spX=x2px(cfg.sp);
  var pvX=(pv!=null&&!isNaN(pv))?x2px(Math.max(xMin,Math.min(xMax,pv))):null;
  var ticks='';
  for(var i=0;i<=4;i++){var v=xMin+i*(xMax-xMin)/4;var tx=x2px(v);ticks+='<line x1="'+tx+'" y1="'+(H-padB)+'" x2="'+tx+'" y2="'+(H-padB+3)+'" stroke="#5a6e8c" stroke-width="0.5"/><text x="'+tx+'" y="'+(H-2)+'" text-anchor="middle" font-family="monospace" font-size="7" fill="#5a6e8c">'+v.toFixed(1)+'</text>';}
  var pvLine=pvX!=null?('<line x1="'+pvX+'" y1="'+(padT-2)+'" x2="'+pvX+'" y2="'+(H-padB+2)+'" stroke="#fff" stroke-width="2"/><circle cx="'+pvX+'" cy="'+(padT+2)+'" r="3" fill="#fff" stroke="#000" stroke-width="0.5"/>'):'';
  document.getElementById('mfBox_'+id).innerHTML='<svg class="mf-svg" viewBox="0 0 '+W+' '+H+'" preserveAspectRatio="none">'
    +'<rect x="0" y="0" width="'+W+'" height="'+(H-padB)+'" fill="#0a1322"/>'
    +'<path d="'+lowPath+'" fill="#3b82f6" fill-opacity="0.45" stroke="#3b82f6" stroke-width="1"/>'
    +'<path d="'+normPath+'" fill="#10b981" fill-opacity="0.45" stroke="#10b981" stroke-width="1"/>'
    +'<path d="'+highPath+'" fill="#ef4444" fill-opacity="0.45" stroke="#ef4444" stroke-width="1"/>'
    +'<line x1="'+spX+'" y1="'+padT+'" x2="'+spX+'" y2="'+(H-padB)+'" stroke="#f59e0b" stroke-width="1.5" stroke-dasharray="2,2"/>'
    +pvLine+ticks+'</svg>';
}
function updateMFMarkers(d){
  if(!MF_DATA)return;
  renderMF('V',d.enV?d.v:null);renderMF('P',d.enP?d.p:null);renderMF('I',d.enI?d.i:null);
  ['V','P','I'].forEach(function(id){var r=document.getElementById('mfRow_'+id);if(r)r.classList.toggle('disabled',!d['en'+id]);});
  if(d.wV!==undefined){document.getElementById('mfWeights').innerHTML='Bobot: V=<b>'+d.wV.toFixed(2)+'</b> · P=<b>'+d.wP.toFixed(2)+'</b> · I=<b>'+d.wI.toFixed(2)+'</b>';}
  var mvV=document.getElementById('mfVal_V');if(mvV)mvV.innerHTML=d.enV?('<b>'+d.v.toFixed(2)+'</b><span>m/s² · SP '+(MF_DATA.V?MF_DATA.V.sp.toFixed(2):'0.50')+'</span>'):'<b>—</b><span>OFF</span>';
  var mvP=document.getElementById('mfVal_P');if(mvP)mvP.innerHTML=d.enP?('<b>'+d.p.toFixed(2)+'</b><span>kPa · SP '+(MF_DATA.P?MF_DATA.P.sp.toFixed(2):'0.50')+'</span>'):'<b>—</b><span>OFF</span>';
  var mvI=document.getElementById('mfVal_I');if(mvI)mvI.innerHTML=d.enI?('<b>'+d.i.toFixed(3)+'</b><span>A · SP '+(MF_DATA.I?MF_DATA.I.sp.toFixed(3):'0.300')+'</span>'):'<b>—</b><span>OFF</span>';
}

// ════════ Polling utama ════════
async function update(){
  var ctrl=new AbortController();var tid=setTimeout(function(){ctrl.abort();},800);
  try{
    var d=await(await fetch('/data',{signal:ctrl.signal})).json();clearTimeout(tid);
    var ev=function(id,v){var e=document.getElementById(id);if(e)e.innerText=v;};
    ev('vib',d.enV?d.v.toFixed(2):'—');
    ev('cur',d.enI?d.i.toFixed(3):'—');
    var mvn=document.getElementById('mcalVibNow');if(mvn)mvn.innerText=d.enV?d.v.toFixed(3)+' m/s²':'— m/s²';
    var ain=document.getElementById('acalINow');if(ain)ain.innerText=d.enI?d.i.toFixed(4)+' A':'— A';
    if(d.acsDone!==undefined){var ab=document.getElementById('acalBadge');if(ab){ab.className='badge '+(d.acsDone?'ok':'def');ab.innerText=d.acsDone?'CALIBRATED':'DEFAULT';}ev('acalOffset',d.acsOff.toFixed(5)+' V');ev('acalGain',d.acsGain.toFixed(5));}
    if(d.mpuDone!==undefined){var mb=document.getElementById('mcalBadge');if(mb){mb.className='badge '+(d.mpuDone?'ok':'def');mb.innerText=d.mpuDone?'CALIBRATED':'DEFAULT';}ev('mcalAx',d.mpuAx.toFixed(5));ev('mcalAy',d.mpuAy.toFixed(5));ev('mcalAz',d.mpuAz.toFixed(5));}
    ev('prs_kpa',d.enP?d.p.toFixed(2):'—');
    ev('prs_bar',d.enP?('≈ '+d.p_bar.toFixed(4)+' Bar'):'≈ — Bar');
    ev('prs_pct',d.enP?(Math.min(d.p/((d.pSp)||0.5)*100,500).toFixed(0)+'% setpoint'):'—');
    ev('pwm',d.pwm);
    var vb2=document.getElementById('vib_base');if(vb2)vb2.innerHTML='baseline gravitasi: <b>'+(d.vbase||9.81).toFixed(2)+' m/s²</b>';
    updateCavUI(d.cav,d.cs);
    if(MF_DATA&&MF_DATA.I&&d.iSpA!==undefined)MF_DATA.I.sp=d.iSpA;
    updateMFMarkers(d);
    if(d.pidMode){var modeTag=document.getElementById('pidModeTag');if(modeTag){var icons={V:'VIBRASI',P:'TEKANAN',I:'ARUS','-':'NONE'};modeTag.className='pid-mode-tag '+(d.pidMode||'-');modeTag.innerText=icons[d.pidMode]||'MODE '+d.pidMode;}}
    if(d.enV)chartPush('vib',d.v);if(d.enI)chartPush('cur',d.i);if(d.enP)chartPush('prs',d.p);
    chartPush('pwm',d.pwm);chartPush('cav',d.cav);if(curPage==='beranda')drawChart();
    var lu=document.getElementById('chartLastUpd');if(lu){var t=new Date();lu.innerText=String(t.getHours()).padStart(2,'0')+':'+String(t.getMinutes()).padStart(2,'0')+':'+String(t.getSeconds()).padStart(2,'0');}
    ev('pidVal',(d.pid>=0?'+':'')+d.pid.toFixed(4));
    var pf=document.getElementById('pidFill');if(pf){pf.style.width=Math.min(Math.abs(d.pid)/5*100,100)+'%';pf.style.background=d.pid>=0?'#10b981':'#f43f5e';}
    ev('pwmTgt',d.pwm);
    var wf=document.getElementById('pwmFill');if(wf){wf.style.width=d.pwm+'%';}
    if(d.pidMode){var units={V:'m/s²',P:'kPa',I:'A','-':'-'};ev('pidSpInfo','SP: '+(d.pidSp||0).toFixed(3)+' '+(units[d.pidMode]||'')+'  ·  PV: '+(d.pidPv||0).toFixed(3)+' '+(units[d.pidMode]||''));}
    if(d.temp!==undefined){
      var tc=d.temp;var tEl=document.getElementById('tmTemp');if(tEl){tEl.innerText=tc.toFixed(1)+'°C';tEl.style.color=tc>=90?'#f43f5e':tc>=80?'#fbbf24':'#34d399';}
      ev('tmCpu',d.cpu+' MHz');
      var thrEl=document.getElementById('tmThr');if(thrEl){thrEl.innerText=d.thr?'AKTIF':'Normal';thrEl.style.color=d.thr?'#fbbf24':'#34d399';}
      ev('tmLps',(d.lps||0).toFixed(0));ev('tmMbps',((d.mbps||0)*1000).toFixed(1)+' KB/s');ev('tmHeap',Math.round((d.heap||0)/1024)+' KB');
      if(d.up!==undefined){var s=Math.floor(d.up/1000),h=Math.floor(s/3600),m=Math.floor((s%3600)/60),sc=s%60;ev('tmUp',String(h).padStart(2,'0')+':'+String(m).padStart(2,'0')+':'+String(sc).padStart(2,'0'));}
      var card=document.getElementById('tmCard');if(card)card.className='tm-card'+(tc>=90?' crit':tc>=80?' warn':'');
      var badge=document.getElementById('tmBadge');if(badge){badge.className='badge'+(tc>=90?' err':tc>=80?' warn':' ok');badge.innerText=tc>=90?'KRITIS':tc>=80?'THROTTLE':'NORMAL';}
    }
    var sb=document.getElementById('sensorBox');if(sb){sb.innerText=d.sens;var norm=d.sens.indexOf('NORMAL')===0;sb.style.color=norm?'var(--grn2)':'var(--red)';sb.style.borderColor=norm?'rgba(16,185,129,.35)':'rgba(244,63,94,.4)';sb.style.background=norm?'rgba(16,185,129,.06)':'rgba(244,63,94,.06)';}
    var box=document.getElementById('status');
    if(box){if(!d.rdy){box.innerText='SISTEM BELUM SIAP';box.className='off';}else if(d.stop){box.innerText='BERHENTI';box.className='stp';}else{box.innerText='BEROPERASI';box.className='run';}}
    if(d.enP!==undefined){['P','V','I'].forEach(function(id){var cb=document.getElementById('s_'+id);if(cb&&cb.checked!==d['en'+id])applySensorState(id,d['en'+id]);});var fm=document.getElementById('fmode');if(fm)fm.innerText='MODE: '+d.fmode+' ('+((d.enP?1:0)+(d.enV?1:0)+(d.enI?1:0))+'/3)';}
    if(d.ctrlPri)reflectCtrl(d.ctrlPri);
    if(d.vtEn!==undefined)reflectVtest({en:d.vtEn,pwm:d.vtPwm});
    if(d.spVcfg!==undefined){ev('warnSpV',d.spVcfg.toFixed(2)+' m/s²');ev('warnSpP',d.spPcfg.toFixed(3)+' kPa');}
    if(d.iSpd!==undefined){var sl=document.getElementById('ispsl');if(sl&&document.activeElement!==sl)sl.value=d.iSpd;ev('ispval',d.iSpd);ev('ispNow',d.iSpd+'%');ev('ispSpA',d.iSpA!==undefined?d.iSpA.toFixed(3)+' A':'— A');}
    if(d.iSpdEff!==undefined){
      var eff=d.iSpdEff;var nom=d.iSpd!==undefined?d.iSpd:50;var effColor=eff<nom?'#fbbf24':'#34d399';
      var effEl=document.getElementById('ispEffNow');if(effEl){effEl.innerText=eff+'%';effEl.style.color=effColor;}
      var redEl=document.getElementById('ispReduction'),boxEl=document.getElementById('ispRedBox');
      if(redEl){var redAmt=nom-eff;if(redAmt>0){redEl.innerText='Kavitasi aktif: -'+redAmt+'% → efektif '+eff+'% dari nominal '+nom+'%';if(boxEl)boxEl.style.color='#fbbf24';}else{redEl.innerText='Normal (100%) — tidak ada reduksi';if(boxEl)boxEl.style.color='#34d399';}}
    }
    if(d.pcalDone!==undefined){var pb=document.getElementById('pcalBadge');if(pb){pb.className='badge '+(d.pcalDone?'ok':'def');pb.innerText=d.pcalDone?'CALIBRATED':'DEFAULT';}ev('pcalZero',d.pcalZero!==undefined?d.pcalZero.toFixed(3)+' kPa':'—');ev('pcalGain',d.pcalGain!==undefined?d.pcalGain.toFixed(4):'—');}
    checkNotif(d);
  }catch(e){clearTimeout(tid);}
  setTimeout(update,200);
}
async function updateLogStatus(){
  try{
    var s=await(await fetch('/logstatus')).json();
    var ld=document.getElementById('logDot');if(ld)ld.className='log-dot'+(s.logging?' active':'');
    var ev=function(id,v){var e=document.getElementById(id);if(e)e.innerText=v;};
    ev('logCountVal',s.count.toLocaleString()+' baris');ev('logSizeVal',s.usedKB+' KB');ev('logHeapVal',Math.round(s.freeHeap/1024)+' KB');ev('logIntVal',s.interval+' ms');
    var pct=Math.min(s.count/s.maxRows*100,100).toFixed(1);
    var lpf=document.getElementById('logProgFill');if(lpf)lpf.style.width=pct+'%';ev('logProgPct',pct+'%');
    var bs=document.getElementById('btnLogStart');if(bs)bs.disabled=s.logging;
    var bst=document.getElementById('btnLogStop');if(bst)bst.disabled=!s.logging;
    var bd=document.getElementById('btnLogDl');if(bd)bd.disabled=s.count===0;
  }catch(e){}
  setTimeout(updateLogStatus,1000);
}
async function logStart(){var iv=document.getElementById('logIntervalInput');await fetch('/log?cmd=start&interval='+(iv?iv.value:500)).catch(function(){});showToast('Logging dimulai','ok');}
async function logStop(){await fetch('/log?cmd=stop').catch(function(){});showToast('Logging dihentikan','warn');}
async function logClear(){if(!await showModal({type:'danger',danger:true,iconSvg:ICO.danger,title:'Hapus Data Log',msg:'Semua data CSV di RAM akan dihapus dan tidak bisa dikembalikan.\n\nLanjutkan?',confirmText:'Hapus'}))return;await fetch('/log?cmd=clear').catch(function(){});showToast('Buffer log dibersihkan','ok');}
function downloadCSV(){var a=document.createElement('a');a.href='/logcsv';a.download='smart_pump_'+Date.now()+'.csv';document.body.appendChild(a);a.click();document.body.removeChild(a);}

// ════════ Kalibrasi tekanan ════════
function showPcalMsg(text,isOk){var m=document.getElementById('pcalMsg');m.className='xmsg show '+(isOk?'ok':'err');m.innerText=text;setTimeout(function(){m.classList.remove('show');},5000);}
async function pcalRequest(qs){try{var r=await(await fetch('/pcal?'+qs)).json();updatePcalDisplay(r);r.ok?showPcalMsg('OK — zero='+r.zero.toFixed(2)+' kPa, gain='+r.gain.toFixed(4),true):showPcalMsg(r.err||'gagal',false);}catch(e){showPcalMsg('kesalahan jaringan',false);}}
function updatePcalDisplay(r){var ev=function(id,v){var e=document.getElementById(id);if(e)e.innerText=v;};ev('pcalZero',(r.zero||0).toFixed(2)+' kPa');ev('pcalGain',(r.gain||1).toFixed(4));ev('pcalMethod',(r.method||'default').toUpperCase());var b=document.getElementById('pcalBadge');if(b){b.className='badge '+(r.done?'ok':'def');b.innerText=r.done?'CALIBRATED':'DEFAULT';}}
async function pcalZero(){if(!await showModal({title:'Set Zero Tekanan',msg:'Pompa harus <b>MATI</b> (PWM=0) dan sensor terbuka ke atmosfer.\n\nLanjutkan kalibrasi zero?',confirmText:'Set Zero'}))return;await pcalRequest('cmd=zero');}
async function pcalSpan(){var v=document.getElementById('pcalRef').value;if(!v||v<1){await showAlert('Masukkan P_ref dari manometer (kPa, min 1).');return;}if(!await showModal({title:'Set Span Tekanan',msg:'Pompa menyala, manometer menunjukkan <b>'+v+' kPa</b>.\n\nLanjutkan?',confirmText:'Set Span'}))return;await pcalRequest('cmd=span&ref='+v);}
async function pcalReset(){if(!await showModal({type:'danger',danger:true,iconSvg:ICO.danger,title:'Reset Kalibrasi Tekanan',msg:'Kembalikan kalibrasi tekanan ke nilai default?',confirmText:'Reset'}))return;await pcalRequest('cmd=reset');}
async function pcalStatus(){try{var r=await(await fetch('/pcal?cmd=status')).json();updatePcalDisplay(r);}catch(e){}}

// ════════ Kalibrasi ACS712 ════════
function showAcalMsg(text,isOk){var m=document.getElementById('acalMsg');m.className='xmsg show '+(isOk?'ok':'err');m.innerText=text;setTimeout(function(){m.classList.remove('show');},6000);}
function updateAcalDisplay(r){var ev=function(id,v){var e=document.getElementById(id);if(e)e.innerText=v;};if(r.offset!==undefined)ev('acalOffset',r.offset.toFixed(5)+' V');if(r.gain!==undefined)ev('acalGain',r.gain.toFixed(5));if(r.iNow!==undefined)ev('acalINow',r.iNow.toFixed(4)+' A');var b=document.getElementById('acalBadge');if(b){b.className='badge '+(r.done?'ok':'def');b.innerText=r.done?'CALIBRATED':'DEFAULT';}}
async function acalRequest(qs){
  var prog=document.getElementById('acalProg'),fill=document.getElementById('acalProgFill');
  prog.className='xprog show';fill.style.width='0%';fill.style.background='#a855f7';
  var t=0;var iv=setInterval(function(){t=Math.min(t+3,90);fill.style.width=t+'%';},80);
  try{var r=await(await fetch('/acscal?'+qs)).json();clearInterval(iv);fill.style.width='100%';setTimeout(function(){prog.className='xprog';},800);updateAcalDisplay(r);r.ok?showAcalMsg('OK '+r.action+' — offset='+r.offset.toFixed(4)+'V gain='+r.gain.toFixed(4),true):showAcalMsg(r.err||'gagal',false);}
  catch(e){clearInterval(iv);prog.className='xprog';showAcalMsg('kesalahan jaringan',false);}
}
async function acalZero(){if(!await showModal({title:'Set Zero ACS712',msg:'Pompa harus <b>MATI</b> (PWM=0), tidak ada arus mengalir.\n\nLanjutkan?',confirmText:'Set Zero'}))return;await acalRequest('cmd=zero');}
async function acalSpan(){var v=document.getElementById('acalRef').value;if(!v||parseFloat(v)<0.05){await showAlert('Masukkan arus referensi dari clamp meter (min 0.05 A).');return;}if(!await showModal({title:'Set Span ACS712',msg:'Pompa menyala dan stabil, clamp meter: <b>'+v+' A</b>.\n\nLanjutkan?',confirmText:'Set Span'}))return;await acalRequest('cmd=span&ref='+v);}
async function acalReset(){if(!await showModal({type:'danger',danger:true,iconSvg:ICO.danger,title:'Reset Kalibrasi ACS712',msg:'Kembalikan ke default (offset=2.5V, gain=1.0)?',confirmText:'Reset'}))return;await acalRequest('cmd=reset');}
async function acalStatus(){try{var r=await(await fetch('/acscal?cmd=status')).json();updateAcalDisplay(r);}catch(e){}}

// ════════ Kalibrasi MPU6050 ════════
function showMcalMsg(text,isOk){var m=document.getElementById('mcalMsg');m.className='xmsg show '+(isOk?'ok':'err');m.innerText=text;setTimeout(function(){m.classList.remove('show');},6000);}
function updateMcalDisplay(r){var ev=function(id,v){var e=document.getElementById(id);if(e)e.innerText=v;};if(r.ax!==undefined)ev('mcalAx',r.ax.toFixed(5));if(r.ay!==undefined)ev('mcalAy',r.ay.toFixed(5));if(r.az!==undefined)ev('mcalAz',r.az.toFixed(5));var b=document.getElementById('mcalBadge');if(b){b.className='badge '+(r.done?'ok':'def');b.innerText=r.done?'CALIBRATED':'DEFAULT';if(r.mpuOK===false){b.className='badge err';b.innerText='MPU NOT FOUND';}}}
async function mcalRequest(qs){
  var prog=document.getElementById('mcalProg'),fill=document.getElementById('mcalProgFill');
  prog.className='xprog show';fill.style.width='0%';fill.style.background='#10b981';
  var t=0;var iv=setInterval(function(){t=Math.min(t+2,90);fill.style.width=t+'%';},100);
  try{var r=await(await fetch('/mpucal?'+qs)).json();clearInterval(iv);fill.style.width='100%';setTimeout(function(){prog.className='xprog';},800);updateMcalDisplay(r);r.ok?showMcalMsg('OK '+r.action+' — ax='+r.ax.toFixed(4)+' ay='+r.ay.toFixed(4)+' az='+r.az.toFixed(4),true):showMcalMsg(r.err||'gagal',false);}
  catch(e){clearInterval(iv);prog.className='xprog';showMcalMsg('kesalahan jaringan',false);}
}
async function mcalStart(){if(!await showModal({title:'Kalibrasi MPU6050',msg:'Pompa harus MATI, sensor tidak bergetar, posisi sudah final.\n\nLanjutkan kalibrasi?',confirmText:'Kalibrasi'}))return;await mcalRequest('cmd=start');}
async function mcalReset(){if(!await showModal({type:'danger',danger:true,iconSvg:ICO.danger,title:'Reset Kalibrasi MPU6050',msg:'Kembalikan offset ke default (0,0,0)?',confirmText:'Reset'}))return;await mcalRequest('cmd=reset');}
async function mcalStatus(){try{var r=await(await fetch('/mpucal?cmd=status')).json();updateMcalDisplay(r);}catch(e){}}

// ════════ ISP / Ctrl / Vtest ════════
var ispTimer=null;
function setIspLive(v){var vv=document.getElementById('ispval');if(vv)vv.innerText=v;clearTimeout(ispTimer);ispTimer=setTimeout(function(){applyIsp(v);},600);}
function applyIsp(v){fetch('/isp?pct='+v).then(function(r){return r.json();}).then(function(j){if(!j.ok)return;var ev=function(id,txt){var e=document.getElementById(id);if(e)e.innerText=txt;};ev('ispNow',j.pct+'%');ev('ispSpA',j.spA.toFixed(3)+' A');ev('ispval',j.pct);var sl=document.getElementById('ispsl');if(sl&&document.activeElement!==sl)sl.value=j.pct;if(MF_DATA&&MF_DATA.I)MF_DATA.I.sp=j.spA;}).catch(function(){});}
function setCtrl(p){fetch('/ctrl?primary='+p).then(function(r){return r.json();}).then(function(j){reflectCtrl(j.primary);}).catch(function(){});}
function setVtest(en){fetch('/vtest?en='+en).then(function(r){return r.json();}).then(function(j){reflectVtest(j);showToast(j.en?'Uji tutup-katup AKTIF':'Uji tutup-katup nonaktif',j.en?'warn':'');}).catch(function(){});}
function setVtestPwm(v){var vv=document.getElementById('vtval');if(vv)vv.innerText=v;fetch('/vtest?pwm='+v).then(function(r){return r.json();}).then(function(j){reflectVtest(j);}).catch(function(){});}
function reflectVtest(j){var e=document.getElementById('vtNow');if(e){e.innerText=j.en?('AKTIF · PWM '+j.pwm):'NONAKTIF';e.style.color=j.en?'var(--amb2)':'var(--dim)';}var s=document.getElementById('vtsl');if(s&&document.activeElement!==s&&j.pwm!=null)s.value=j.pwm;var vv=document.getElementById('vtval');if(vv&&j.pwm!=null)vv.innerText=j.pwm;}
function reflectCtrl(p){var names={V:'VIBRASI',P:'TEKANAN',I:'ARUS','-':'-'};var el=document.getElementById('ctrlNow');if(el)el.innerText='PID: '+(names[p]||p);var al=document.getElementById('pidActLbl');if(al)al.innerText=names[p]||p;}

document.addEventListener('DOMContentLoaded',function(){
  applyBerandaExtras();renderNotif();showPage('beranda');
  refreshSensorUI();loadMFData();pcalStatus();acalStatus();mcalStatus();update();updateLogStatus();
});
</script>
</body></html>
)rawliteral";