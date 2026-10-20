#pragma once
#include <Arduino.h>
// Весь интерфейс встроен в flash; внешних CDN и файлов нет.
static const char INDEX_HTML[] PROGMEM = R"CCHTML(<!doctype html>
<html lang="ru"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Climate Camera v1.3.2</title><link rel="icon" type="image/png" href="/favicon-96x96.png" sizes="96x96"><link rel="icon" type="image/png" href="/favicon-32x32.png" sizes="32x32"><link rel="icon" type="image/png" href="/favicon-16x16.png" sizes="16x16"><link rel="shortcut icon" href="/favicon.ico"><link rel="apple-touch-icon" sizes="180x180" href="/apple-touch-icon.png"><meta name="apple-mobile-web-app-title" content="КлиматКамера"><meta name="theme-color" content="#0a1220"><link rel="manifest" href="/site.webmanifest">
<style>
:root{color-scheme:dark;--bg:#0a1220;--card:#152236;--muted:#9bb1c9;--accent:#55d4ce;--line:#2a3c54}*{box-sizing:border-box}body{margin:0;background:var(--bg);color:#eef5fc;font:15px system-ui,Arial,sans-serif}main{max-width:1040px;margin:auto;padding:26px 18px}header{display:flex;align-items:center;justify-content:space-between;gap:16px;margin-bottom:22px}h1{font-size:25px;margin:0 0 5px}h2{font-size:18px;margin:0 0 16px}small,.muted{color:var(--muted)}.badge{padding:8px 12px;border-radius:24px;background:#20344d;font-size:13px}.metrics{display:grid;grid-template-columns:repeat(3,1fr);gap:14px}.card{background:var(--card);border:1px solid var(--line);border-radius:16px;padding:20px;margin-bottom:16px}.value{font-size:38px;font-weight:700;letter-spacing:-1px;margin:9px 0}.unit{font-size:18px;color:var(--muted)}.grid{display:grid;grid-template-columns:1fr 1fr;gap:16px}.row{display:flex;justify-content:space-between;gap:14px;padding:10px 0;border-bottom:1px solid var(--line)}.row:last-child{border:0}.on{color:#6ee7b7}.off{color:var(--muted)}.warn{color:#fbbf24}.fields{display:grid;grid-template-columns:1fr 1fr;gap:12px}label{display:block;font-size:13px;color:var(--muted)}input,select{width:100%;margin-top:6px;padding:10px 11px;border-radius:9px;border:1px solid #3d5370;background:#0d192a;color:#fff;font:inherit}button,.link{display:inline-block;border:0;border-radius:9px;background:var(--accent);color:#082221;padding:11px 15px;font-weight:650;cursor:pointer;text-decoration:none;font-size:14px}button.secondary,.link{background:#263c55;color:#eef5fc}button.danger{background:#57343a;color:#ffdbe0}button.small{padding:7px 11px;font-size:13px}button:disabled{opacity:.45;cursor:default}.actions{display:flex;flex-wrap:wrap;gap:9px;margin-top:16px}.message{padding:13px 16px;background:#21334b;border-radius:11px;margin-bottom:16px;min-height:46px}.foot{font-size:12px;line-height:1.7;margin-top:12px}details{margin-top:12px}summary{cursor:pointer;color:var(--muted);padding:8px 0}@media(max-width:650px){.grid{grid-template-columns:1fr;gap:0}.metrics{grid-template-columns:1fr 1fr}.metrics .card:last-child{grid-column:1/-1}.value{font-size:33px}header{align-items:flex-start}.fields{gap:10px}main{padding:18px 12px}}
[hidden]{display:none!important}.tabs{display:flex;gap:10px;margin:18px 0;flex-wrap:wrap}.tabs button{width:auto}.tabs button[aria-selected="true"]{outline:2px solid #93c5fd}.event-tools{display:flex;gap:12px;align-items:center;flex-wrap:wrap}.event-tools label{display:flex;gap:8px;align-items:center}.event-tools input{width:auto}.event-table{width:100%;border-collapse:collapse;margin-top:14px}.event-table th,.event-table td{padding:10px 8px;text-align:left;border-bottom:1px solid #304457;vertical-align:top}.event-table td:last-child{overflow-wrap:anywhere}.event-table th:first-child,.event-table td:first-child{white-space:nowrap}.scroll{overflow-x:auto}.diag{overflow-wrap:anywhere}
.prod-grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(300px,1fr));gap:14px;margin-top:16px}.prod-card{background:#101c2e;border:1px solid var(--line);border-radius:14px;padding:16px;display:flex;flex-direction:column;justify-content:space-between}.prod-head{display:flex;justify-content:space-between;align-items:center;margin-bottom:10px;border-bottom:1px solid var(--line);padding-bottom:8px}.prod-title{font-weight:700;font-size:16px;color:var(--accent)}.prod-loss{font-size:20px;font-weight:700}.modal-backdrop{position:fixed;inset:0;background:rgba(4,9,18,.78);display:flex;align-items:center;justify-content:center;padding:16px;z-index:50}.modal{background:var(--card);border:1px solid var(--line);border-radius:16px;max-width:560px;width:100%;padding:20px;max-height:88vh;overflow-y:auto}</style></head><body><main>
<header><div><h1>Климатическая камера</h1><small>ESP32 · ILI9341 320×240 · <span id="version">v1.3.2</span> · <span id="clockNow">--:--:--</span></small></div><div style="display:flex;gap:8px;align-items:center;flex-wrap:wrap"><span id="connection" class="badge">Подключение…</span><button id="switchUser" type="button" class="secondary" style="padding:7px 11px;font-size:12px">Сменить аккаунт</button><button id="logoutBtn" type="button" class="danger" style="padding:7px 11px;font-size:12px">Выйти</button></div></header>
<div id="message" class="message" role="status">Чтение состояния контроллера…</div>
<nav class="tabs" role="tablist" aria-label="Разделы"><button id="controlTab" type="button" role="tab" aria-selected="true" aria-controls="controlPanel">Управление</button><button id="productsTab" type="button" class="secondary" role="tab" aria-selected="false" aria-controls="productsPanel">Контроль за весом продукта</button><button id="eventsTab" type="button" class="secondary" role="tab" aria-selected="false" aria-controls="eventsPanel">Журнал событий</button></nav>
<section id="controlPanel" role="tabpanel" aria-labelledby="controlTab">
<section class="metrics"><article class="card"><small>ТЕМПЕРАТУРА</small><div class="value"><span id="temp">—</span> <span class="unit">°C</span></div><small id="tempGoal">Уставка —</small></article><article class="card"><small>ВЛАЖНОСТЬ</small><div class="value"><span id="hum">—</span> <span class="unit">%</span></div><small id="humGoal">Уставка —</small></article><article class="card"><small>ВЕНТИЛЯТОР</small><div class="value"><span id="rpm">—</span> <span class="unit">RPM</span></div><small id="fanGoal">Уставка —</small></article></section>
<div class="grid"><section class="card"><h2>Состояние оборудования</h2><div class="row"><span>Система</span><b id="system">—</b></div><div class="row"><span>HTU21D (GY-211)</span><b id="sensor">—</b></div><div class="row"><span>Возраст измерений</span><b id="sensorAge">—</b></div><p id="sensorDiagnostic" class="foot muted diag"></p><div class="row"><span>IN1 · Вентилятор</span><b id="fan">—</b></div><div class="row"><span>IN2 · Увлажнитель</span><b id="humid">—</b></div><div class="row"><span>IN3 · Не используется</span><span class="off">Выключен</span></div><div class="row"><span>IN4 · Компрессор</span><b id="fridge">—</b></div><div class="row"><span>Тахометр</span><b id="tach">—</b></div><div class="row"><span>Время работы</span><span id="uptime">—</span></div><div class="actions"><button data-action="on">Включить</button><button data-action="off" class="danger">Выключить</button><button data-action="ack" class="secondary">Сбросить аварию</button></div><p class="foot muted">Компрессор: задержка 5 мин, минимальный ход 2 мин, максимальный ход 40 мин. Увлажнитель: 20 с работы / 120 с паузы. Аварийная остановка имеет приоритет.</p><a class="link" href="/update">Обновление OTA →</a></section>
<section class="card"><h2>Настройки климата</h2><form id="settings"><div class="fields"><label>Температура, °C<input name="tempTarget" type="number" step="0.1" min="2" max="30" required></label><label>Гистерезис, °C<input name="tempHyst" type="number" step="0.1" min="0.2" max="5" required></label><label>Влажность, %<input name="humTarget" type="number" step="0.1" min="30" max="98" required></label><label>Гистерезис, %<input name="humHyst" type="number" step="0.1" min="1" max="10" required></label><label>Работа вентилятора, мин<input name="fanWorkMin" type="number" min="1" max="180" required></label><label>Пауза вентилятора, мин<input name="fanPauseMin" type="number" min="0" max="360" required></label><label>Целевые обороты, RPM<input name="fanTargetRPM" type="number" step="10" min="300" max="1500" required></label></div><details><summary>Калибровка и ПИ-регулятор</summary><div class="fields"><label>Поправка температуры<input name="tempOffset" type="number" step="0.1" min="-5" max="5" required></label><label>Поправка влажности<input name="humOffset" type="number" step="0.1" min="-15" max="15" required></label><label>Kp<input name="pidKp" type="number" step="0.001" min="0" max="1" required></label><label>Ki<input name="pidKi" type="number" step="0.001" min="0" max="1" required></label></div></details><div class="actions"><button type="submit">Применить и сохранить</button><button id="reload" type="button" class="secondary">Перечитать</button></div></form><p class="foot muted">Запись в NVS выполняется только по команде. Редактирование энкодером блокирует изменение настроек через браузер. После перезагрузки автозапуск включён, но защиты остаются активными.</p></section></div>
<p class="foot muted" id="networkInfo">Резервная точка доступа: 192.168.4.1 · climate-camera.local</p></section>

<section id="productsPanel" class="card" role="tabpanel" aria-labelledby="productsTab" hidden>
<div style="display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:12px"><div><h2 style="margin:0 0 4px">Контроль за весом продукта (до 30 слотов)</h2><small class="muted" id="productsSummary">Загрузка данных о продуктах…</small></div><div style="display:flex;gap:9px;flex-wrap:wrap"><button type="button" id="btnShowAddProduct">+ Загрузить новый продукт</button><button type="button" id="btnRefreshProducts" class="secondary">Обновить</button></div></div>
<form id="addProductForm" class="card" style="margin-top:16px;background:#0e1a2b" hidden>
<h2>Загрузить новый продукт в камеру</h2>
<div class="fields">
<label>Номер продукта (1..30)<select id="addProdSlot" name="id"></select></label>
<label>Первоначальный вес, г<input id="addProdWeight" name="initialWeight" type="number" step="0.1" min="1" max="100000" placeholder="Например, 1250" required></label>
<label>Дата закладки<input id="addProdDate" name="startDate" type="date" required></label>
<label>Целевая усушка, % (для прогноза готовности)<input id="addProdTarget" name="targetLossPct" type="number" step="0.5" min="5" max="80" value="38"></label>
</div>
<div class="actions"><button type="submit">Сохранить продукт</button><button type="button" id="btnCancelAddProduct" class="secondary">Отмена</button></div>
</form>
<div id="productsList" class="prod-grid"></div>
</section>

<div id="historyModal" class="modal-backdrop" hidden>
<div class="modal">
<div style="display:flex;justify-content:space-between;align-items:center;gap:10px;margin-bottom:12px"><h2 id="historyTitle" style="margin:0">История контрольных взвешиваний</h2><button type="button" id="btnCloseHistory" class="secondary small">Закрыть</button></div>
<p id="historyMeta" class="muted foot" style="margin-top:0"></p>
<div class="scroll"><table class="event-table"><thead><tr><th>№</th><th>Дата</th><th>Вес, г</th><th>Потеря, г</th><th>Усушка, %</th></tr></thead><tbody id="historyRows"></tbody></table></div>
</div>
</div>

<section id="eventsPanel" class="card" role="tabpanel" aria-labelledby="eventsTab" hidden><h2>Журнал событий</h2><p class="muted">Последние 40 событий в оперативной памяти с привязкой к реальной дате и времени. Новые события сверху.</p><div class="event-tools"><button type="button" id="eventsRefresh">Обновить</button><button type="button" id="eventsDownload" class="secondary">Скачать TXT</button><label><input id="eventsAuto" type="checkbox" checked> Автообновление · 2 с</label></div><p id="eventsInfo" class="foot muted" role="status">Откройте вкладку для загрузки журнала.</p><div class="scroll"><table class="event-table"><thead><tr><th>Дата и время</th><th>Событие</th></tr></thead><tbody id="eventsRows"></tbody></table></div></section>
</main>
<script>
'use strict';
let token='',initialized=false,dirty=false,pending=false,lastState=null,messageUntil=0;
const $=id=>document.getElementById(id), form=$('settings');
function message(text,error=false){$('message').textContent=text;$('message').style.color=error?'#fecaca':'#eef5fc';messageUntil=Date.now()+5000;}
function fill(c){for(const el of form.elements){if(el.name && Object.hasOwnProperty.call(c,el.name)) el.value=c[el.name];}dirty=false;initialized=true;}
function flag(id,on,a='Работает',b='Выключен'){$(id).textContent=on?a:b;$(id).className=on?'on':'off';}
async function refresh(force=false){
 try{
 const r=await fetch('/api/status',{cache:'no-store'});if(!r.ok)throw new Error('HTTP '+r.status);const d=await r.json();lastState=d;token=d.token;
 $('version').textContent='v'+(d.version||'1.3.2');
 if($('clockNow'))$('clockNow').textContent=d.time?(d.date?(d.date+' · '+d.time):d.time):'NTP…';
 $('connection').textContent=(d.wifiConnected?('Wi-Fi '+d.rssi+' дБм'):'Резервная точка AP')+(d.readOnly?' · Мониторинг':'');
 const tVal=(typeof d.temp==='number')?d.temp:null;
 const hVal=(typeof d.hum==='number')?d.hum:((typeof d.humidity==='number')?d.humidity:null);
 const rVal=(typeof d.rpm==='number')?d.rpm:((typeof d.fanRPM==='number')?d.fanRPM:0);
 $('temp').textContent=tVal===null?'—':tVal.toFixed(1);
 $('hum').textContent=hVal===null?'—':hVal.toFixed(1);
 $('rpm').textContent=Math.round(rVal);
 if(d.config){
  $('tempGoal').textContent='Уставка '+Number(d.config.tempTarget).toFixed(1)+' °C';
  $('humGoal').textContent='Уставка '+Number(d.config.humTarget).toFixed(1)+' %';
  $('fanGoal').textContent='Уставка '+d.config.fanTargetRPM+' RPM';
 }
 flag('system',d.enabled,'Включена','Выключена');flag('sensor',d.sensorOk,'Данные корректны','Аварийный таймер');if(d.sensorDegraded){$('sensor').textContent='Сбой '+d.sensorFailures+'/3 · старые данные';$('sensor').className='warn';}$('sensorAge').textContent=d.sensorAgeSec===null?'Нет измерений':d.sensorAgeSec+' с';$('sensorDiagnostic').textContent=d.sensorError||'';flag('fan',d.fan);flag('humid',d.humid);flag('fridge',d.fridge);if(!d.fridge && d.fridgeWaitSec){$('fridge').textContent='Пауза '+d.fridgeWaitSec+' с';$('fridge').className='warn';}flag('tach',!d.fanTachFault,'Без аварии','АВАРИЯ');$('uptime').textContent=Math.floor(d.uptimeSec/3600)+' ч '+Math.floor(d.uptimeSec%3600/60)+' мин';$('networkInfo').textContent='STA: '+d.staIP+' (GW: '+d.gatewayIP+') · MAC: '+d.mac+' · AP: '+d.apIP+' · climate-camera.local';
 if(!initialized||force||(!dirty && document.activeElement.tagName!=='INPUT'))fill(d.config);
 for(const b of document.querySelectorAll('#controlPanel button'))b.disabled=pending||d.readOnly||(!token)||(d.otaMode && b.dataset.action!=='off');for(const inp of form.elements)if(inp.tagName==='INPUT')inp.readOnly=!!d.readOnly;
 if(d.otaMode){$('message').textContent='OTA: автоматика остановлена, все выходы выключены.';}else if(Date.now()>messageUntil){$('message').textContent=d.uiEditing?'Локальное редактирование: закончите работу с энкодером.':(!d.sensorOk?'Авария датчика HTU21D (GY-211)! Аварийный режим: холодильник 4 мин работа / 10 мин пауза, вентилятор штатно, увлажнитель отключён.':(d.sensorDegraded?'Кратковременный сбой HTU21D (GY-211) ('+d.sensorFailures+'/3). Используются последние корректные измерения.':(d.fanTachFault?'Авария вентилятора. Устраните причину перед запуском.':'Контроллер на связи. Состояние обновляется автоматически.')));}
 }catch(e){$('connection').textContent='Нет связи';if(Date.now()>messageUntil)$('message').textContent='Не удалось получить данные. Проверьте подключение: '+e.message;}
}
async function post(path,body){if(!token)throw new Error('Нет токена: перечитайте состояние');pending=true;try{const r=await fetch(path,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded','X-Climate-Token':token},body});const d=await r.json();if(!r.ok||!d.ok)throw new Error(d.message||('HTTP '+r.status));message(d.message);return d;}finally{pending=false;}}
form.addEventListener('input',()=>dirty=true);form.addEventListener('submit',async e=>{e.preventDefault();const body=new URLSearchParams(new FormData(form));body.set('save','1');try{await post('/api/config',body);dirty=false;await refresh(true);}catch(e){message(e.message,true);}});
for(const b of document.querySelectorAll('[data-action]'))b.addEventListener('click',async()=>{try{await post('/api/control',new URLSearchParams({action:b.dataset.action}));await refresh();}catch(e){message(e.message,true);}});
let activeTab='control',eventBusy=false,eventData=null,productsData=null;
function sinceBoot(seconds){const h=Math.floor(seconds/3600),m=Math.floor(seconds%3600/60),s=Math.floor(seconds%60);return h+':'+String(m).padStart(2,'0')+':'+String(s).padStart(2,'0');}
function fmtEventTime(entry){return (entry.dateTime&&entry.dateTime.length)?entry.dateTime:('от старта '+sinceBoot(entry.uptimeSec));}
async function refreshEvents(){
 if(eventBusy)return;eventBusy=true;$('eventsRefresh').disabled=true;
 try{const r=await fetch('/api/events',{cache:'no-store'});if(!r.ok)throw new Error('HTTP '+r.status);const d=await r.json();eventData=d;const rows=$('eventsRows');rows.replaceChildren();
 for(const entry of [...d.events].reverse()){const row=document.createElement('tr'),time=document.createElement('td'),msg=document.createElement('td');time.textContent=fmtEventTime(entry);msg.textContent=entry.message;row.append(time,msg);rows.append(row);}
 if(!d.events.length){const row=document.createElement('tr'),cell=document.createElement('td');cell.colSpan=2;cell.textContent='Журнал пуст';row.append(cell);rows.append(row);}
 $('eventsInfo').textContent='Записей: '+d.count+'/'+d.capacity+' · Контроллер работает '+sinceBoot(d.uptimeSec)+' · v'+d.version;
 }catch(e){$('eventsInfo').textContent='Не удалось прочитать журнал: '+e.message;}finally{eventBusy=false;$('eventsRefresh').disabled=false;}
}

function isoToday(){
 if(lastState&&lastState.isoDate)return lastState.isoDate;
 if(productsData&&productsData.today)return productsData.today;
 const n=new Date(),y=n.getFullYear(),m=String(n.getMonth()+1).padStart(2,'0'),d=String(n.getDate()).padStart(2,'0');
 return y+'-'+m+'-'+d;
}
function fmtDateRu(iso){
 if(!iso||iso.length!==10)return iso||'—';
 return iso.slice(8,10)+'.'+iso.slice(5,7)+'.'+iso.slice(0,4);
}
function calcReadyDate(pr){
 const w0=pr.initialWeight,targetPct=(pr.targetLossPct||38),wTarget=w0*(1-targetPct/100);
 const ws=pr.weighings||[];
 if(!ws.length||w0<=0)return 'Недостаточно замеров';
 const last=ws[ws.length-1];
 const curLossPct=((w0-last.weight)/w0)*100;
 if(curLossPct>=targetPct)return 'Готов (достигнуто '+curLossPct.toFixed(1)+'%)';
 const t0=Date.parse(pr.startDate+'T00:00:00Z'),tLast=Date.parse(last.date+'T00:00:00Z');
 if(!isFinite(t0)||!isFinite(tLast))return 'Ожидание следующего взвешивания';
 let days=(tLast-t0)/86400000;
 if(days<=0&&ws.length>1)days=ws.length-1;
 const lost=w0-last.weight;
 if(days<=0||lost<=0)return 'Взвесьте повторно через 1+ дн.';
 const ratePerDay=lost/days;
 const remGrams=last.weight-wTarget;
 const remDays=Math.ceil(remGrams/ratePerDay);
 if(!isFinite(remDays)||remDays>730)return 'Слишком малая скорость усушки';
 const readyMs=tLast+remDays*86400000;
 const rd=new Date(readyMs);
 const iso=rd.getUTCFullYear()+'-'+String(rd.getUTCMonth()+1).padStart(2,'0')+'-'+String(rd.getUTCDate()).padStart(2,'0');
 return '~'+fmtDateRu(iso)+' (через '+remDays+' дн.)';
}
function openHistoryModal(pr){
 $('historyTitle').textContent='Продукт №'+pr.id+' · Все контрольные взвешивания';
 const w0=pr.initialWeight||0;
 $('historyMeta').textContent='Дата закладки: '+fmtDateRu(pr.startDate)+' · Начальный вес: '+w0.toFixed(1)+' г · Целевая усушка: '+(pr.targetLossPct||38).toFixed(1)+'%';
 const rows=$('historyRows');rows.replaceChildren();
 const ws=pr.weighings||[];
 ws.forEach((w,idx)=>{
  const tr=document.createElement('tr');
  const lossG=w0-w.weight,lossPct=w0>0?(lossG/w0*100):0;
  for(const txt of [String(idx+1)+(idx===0?' (закладка)':''),fmtDateRu(w.date),w.weight.toFixed(1),lossG.toFixed(1),lossPct.toFixed(1)+'%']){
   const td=document.createElement('td');td.textContent=txt;tr.append(td);
  }
  rows.append(tr);
 });
 $('historyModal').hidden=false;
}
$('btnCloseHistory').addEventListener('click',()=>{$('historyModal').hidden=true;});
$('historyModal').addEventListener('click',e=>{if(e.target===$('historyModal'))$('historyModal').hidden=true;});

async function refreshProducts(){
 try{
  const r=await fetch('/api/products',{cache:'no-store'});if(!r.ok)throw new Error('HTTP '+r.status);
  const d=await r.json();productsData=d;
  const active=(d.products||[]).filter(p=>p.active);
  $('productsSummary').textContent='Активных продуктов в камере: '+active.length+' из 30';
  const sel=$('addProdSlot');sel.replaceChildren();
  for(const p of (d.products||[])){
   if(!p.active){
    const opt=document.createElement('option');opt.value=String(p.id);opt.textContent='Продукт №'+p.id+' (свободен)';sel.append(opt);
   }
  }
  $('btnShowAddProduct').disabled=!sel.options.length||!!(lastState&&lastState.readOnly);
  const list=$('productsList');list.replaceChildren();
  if(!active.length){
   const empty=document.createElement('p');empty.className='muted';empty.textContent='В камере пока нет загруженных продуктов. Нажмите «+ Загрузить новый продукт», чтобы добавить продукт №1.';
   list.append(empty);return;
  }
  for(const pr of active){
   const card=document.createElement('article');card.className='prod-card';
   const ws=pr.weighings||[];
   const last=ws.length?ws[ws.length-1]:{date:pr.startDate,weight:pr.initialWeight};
   const lossG=pr.initialWeight-last.weight;
   const lossPct=pr.initialWeight>0?((lossG/pr.initialWeight)*100):0;
   const targetPct=pr.targetLossPct||38;
   card.innerHTML=
    '<div><div class="prod-head"><span class="prod-title">Продукт №'+pr.id+'</span><span class="prod-loss '+(lossPct>=targetPct?'on':'')+'">Усушка: '+lossPct.toFixed(1)+'%</span></div>'+
    '<div class="row"><span>Дата закладки</span><b>'+fmtDateRu(pr.startDate)+'</b></div>'+
    '<div class="row"><span>Первоначальный вес</span><b>'+pr.initialWeight.toFixed(1)+' г</b></div>'+
    '<div class="row"><span>Последнее взвешивание ('+fmtDateRu(last.date)+')</span><b>'+last.weight.toFixed(1)+' г (−'+lossG.toFixed(1)+' г)</b></div>'+
    '<div class="row"><span>Примерная дата готовности ('+targetPct.toFixed(0)+'%)</span><b>'+calcReadyDate(pr)+'</b></div></div>';
   const formWeigh=document.createElement('form');
   formWeigh.style.marginTop='12px';
   formWeigh.innerHTML=
    '<label>Новое контрольное взвешивание, г (дата '+fmtDateRu(isoToday())+' подставится автоматически)<div style="display:flex;gap:8px;margin-top:6px"><input type="number" step="0.1" min="1" max="100000" placeholder="Вес в граммах" required style="margin-top:0"><button type="submit" class="small">Внести вес</button></div></label>';
   formWeigh.addEventListener('submit',async ev=>{
    ev.preventDefault();
    const inp=formWeigh.querySelector('input');
    const body=new URLSearchParams({id:String(pr.id),weight:inp.value,date:isoToday()});
    try{await post('/api/products/weigh',body);inp.value='';await refreshProducts();}catch(err){message(err.message,true);}
   });
   const actions=document.createElement('div');actions.className='actions';
   const btnHist=document.createElement('button');btnHist.type='button';btnHist.className='secondary small';
   btnHist.textContent='История взвешиваний ('+ws.length+')';
   btnHist.addEventListener('click',()=>openHistoryModal(pr));
   const btnRem=document.createElement('button');btnRem.type='button';btnRem.className='danger small';
   btnRem.textContent='Извлечь продукт';
   btnRem.addEventListener('click',async()=>{
    if(!confirm('Извлечь Продукт №'+pr.id+' и полностью очистить данные этого слота?'))return;
    try{await post('/api/products/remove',new URLSearchParams({id:String(pr.id)}));await refreshProducts();}catch(err){message(err.message,true);}
   });
   actions.append(btnHist,btnRem);
   card.append(formWeigh,actions);
   list.append(card);
  }
 }catch(e){$('productsSummary').textContent='Ошибка чтения продуктов: '+e.message;}
}
$('btnShowAddProduct').addEventListener('click',()=>{
 $('addProdDate').value=isoToday();
 $('addProductForm').hidden=false;
 $('addProdWeight').focus();
});
$('btnCancelAddProduct').addEventListener('click',()=>{$('addProductForm').hidden=true;});
$('btnRefreshProducts').addEventListener('click',refreshProducts);
$('addProductForm').addEventListener('submit',async e=>{
 e.preventDefault();
 const body=new URLSearchParams({
  id:$('addProdSlot').value,
  initialWeight:$('addProdWeight').value,
  startDate:$('addProdDate').value||isoToday(),
  targetLossPct:$('addProdTarget').value||'38'
 });
 try{
  await post('/api/products/add',body);
  $('addProdWeight').value='';
  $('addProductForm').hidden=true;
  await refreshProducts();
 }catch(err){message(err.message,true);}
});

function selectTab(tab){
 activeTab=tab;
 $('controlPanel').hidden=tab!=='control';
 $('productsPanel').hidden=tab!=='products';
 $('eventsPanel').hidden=tab!=='events';
 $('controlTab').setAttribute('aria-selected',String(tab==='control'));
 $('productsTab').setAttribute('aria-selected',String(tab==='products'));
 $('eventsTab').setAttribute('aria-selected',String(tab==='events'));
 if(tab==='events')refreshEvents();
 if(tab==='products')refreshProducts();
}
$('controlTab').addEventListener('click',()=>selectTab('control'));
$('productsTab').addEventListener('click',()=>selectTab('products'));
$('eventsTab').addEventListener('click',()=>selectTab('events'));
$('eventsRefresh').addEventListener('click',refreshEvents);
$('eventsDownload').addEventListener('click',async()=>{await refreshEvents();if(!eventData){$('eventsInfo').textContent='Сначала загрузите журнал.';return;}const lines=['ESP32-Climate-Camera v'+eventData.version,'Журнал событий (дата и время синхронизированы по NTP).',''];for(const e of eventData.events)lines.push('['+fmtEventTime(e)+'] #'+e.id+' '+e.message);const url=URL.createObjectURL(new Blob(['\ufeff'+lines.join('\r\n')+'\r\n'],{type:'text/plain;charset=utf-8'}));const a=document.createElement('a');a.href=url;a.download='ClimateCamera-v'+eventData.version+'-events.txt';a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);});
$('switchUser').addEventListener('click',()=>{const target=(lastState&&lastState.readOnly)?'admin':'viewer';fetch('/api/reauth?user='+target,{cache:'no-store',credentials:'same-origin'}).then(()=>location.reload()).catch(()=>location.reload());});
$('logoutBtn').addEventListener('click',()=>{fetch('/api/logout',{cache:'no-store',credentials:'same-origin'}).finally(()=>{message('Сессия завершена. Обновите страницу для нового входа.');});});
$('reload').addEventListener('click',()=>refresh(true));async function poll(){await refresh();if(activeTab==='events'&&$('eventsAuto').checked)await refreshEvents();setTimeout(poll,2000);}poll();
</script></body></html>)CCHTML";
static const char UPDATE_HTML[] PROGMEM = R"CCHTML(<!doctype html><html lang="ru"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>OTA · Climate Camera v1.3.2</title><link rel="icon" type="image/png" href="/favicon-96x96.png" sizes="96x96"><link rel="icon" type="image/png" href="/favicon-32x32.png" sizes="32x32"><link rel="icon" type="image/png" href="/favicon-16x16.png" sizes="16x16"><link rel="shortcut icon" href="/favicon.ico"><link rel="apple-touch-icon" sizes="180x180" href="/apple-touch-icon.png"><meta name="apple-mobile-web-app-title" content="КлиматКамера"><meta name="theme-color" content="#0a1220"><link rel="manifest" href="/site.webmanifest"><style>body{margin:0;background:#0a1220;color:#eef5fc;font:15px system-ui,Arial;padding:30px 16px;line-height:1.6}main{max-width:560px;margin:auto;background:#152236;border:1px solid #2a3c54;padding:26px;border-radius:18px}h1{font-size:24px}input{display:block;width:100%;margin:22px 0}button{border:0;border-radius:9px;background:#55d4ce;color:#082221;padding:12px 18px;font-weight:bold;cursor:pointer}button:disabled{opacity:.4}progress{width:100%;margin-top:22px;height:14px}a{color:#55d4ce}.note{color:#b5c7db}#status{min-height:54px}</style></head><body><main><a href="/">← К состоянию камеры</a><h1>Обновление прошивки</h1><p class="note">v1.3.2 · Перед записью ESP32 остановит автоматику и отключит все реле. Нужен <b>бинарный файл приложения</b> для этой платы и схемы разделов с OTA.</p><p class="note">Не загружайте merged/full, bootloader или partitions. Не отключайте питание. Проверьте совместимость образа ILI9341 320×240.</p><form id="form"><input id="file" type="file" accept=".bin" required><button id="send" disabled>Загрузить приложение</button></form><progress id="progress" max="100" value="0"></progress><p id="status" role="status">Проверка соединения…</p><p class="note">После успешной записи контроллер перезагрузится. При ошибке перезагрузки нет, система остаётся выключенной до ручного запуска. HTTP Basic Auth не шифрует соединение: используйте только доверенную локальную сеть.</p></main><script>
'use strict';
let token='';
const status=document.getElementById('status'),button=document.getElementById('send'),progress=document.getElementById('progress');
async function getFreshOtaToken(){
  const r=await fetch('/api/status?otaToken='+Date.now(),{cache:'no-store',credentials:'same-origin'});
  if(!r.ok)throw new Error('HTTP '+r.status);
  const d=await r.json();
  if(d.readOnly||typeof d.token!=='string'||d.token.length!==32)throw new Error('Требуется вход под учётной записью администратора');
  return d.token;
}
getFreshOtaToken().then(t=>{token=t;button.disabled=false;status.textContent='Готово к загрузке. Сначала измените заводские пароли.';}).catch(e=>status.textContent='Нет связи: '+e.message);
document.getElementById('form').addEventListener('submit',async e=>{
  e.preventDefault();
  const file=document.getElementById('file').files[0];
  if(!file||!file.name.toLowerCase().endsWith('.bin')){status.textContent='Выберите файл .bin приложения.';return;}
  if(!confirm('Остановить климатическую автоматику и начать обновление?'))return;
  button.disabled=true;
  status.textContent='Проверка сессии и получение токена…';
  try{token=await getFreshOtaToken();}catch(err){status.textContent='Обновление не начато: '+err.message;button.disabled=false;return;}
  status.textContent='Остановка выходов и загрузка…';
  const data=new FormData();
  data.append('update',file);
  const x=new XMLHttpRequest();
  x.open('POST','/update');
  x.setRequestHeader('X-Climate-Token',token);
  x.timeout=180000;
  x.upload.onprogress=ev=>{if(ev.lengthComputable)progress.value=100*ev.loaded/ev.total;};
  x.onload=()=>{
    let d;try{d=JSON.parse(x.responseText);}catch(err){d={ok:false,message:'Неожиданный ответ сервера'};}
    status.textContent=d.message;
    if(x.status===200&&d.ok){progress.value=100;status.textContent+=' Контроллер перезагружается — через 15–20 секунд откройте главную страницу.';}
    else{button.disabled=false;}
  };
  x.onerror=()=>{status.textContent='Соединение закрыто до получения ответа ESP32 (возможна перезагрузка после записи). Подождите 20 секунд и откройте главную страницу для проверки версии.';button.disabled=false;};
  x.ontimeout=()=>{status.textContent='Превышено время ожидания ответа (180 с). Проверьте состояние контроллера и Serial-лог.';button.disabled=false;};
  x.send(data);
});
</script></body></html>)CCHTML";
