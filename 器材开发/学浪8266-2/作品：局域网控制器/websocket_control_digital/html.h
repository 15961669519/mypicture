const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>8266物联网控制器（1024编程实验室）</title>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1,minimum-scale=1,maximum-scale=1,user-scalable=no" />
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  h2{
    font-size: 1.5rem;
    font-weight: bold;
    color: #143642;
  }
  .topnav {
    overflow: hidden;
    background-color: #143642;
  }
  body {
    margin: 0;
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  .card {
    background-color: #F8F7F9;;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }
  .button {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #0f8b8d;
    border: none;
    border-radius: 5px;
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    -webkit-tap-highlight-color: rgba(0,0,0,0);
   }
   /*.button:hover {background-color: #0f8b8d}*/
   .button:active {
     /* background-color: #0f8b8d; */
     box-shadow: 2 2px #CDCDCD;
     transform: translateY(2px);
   }
   .button_small{
    padding: 13px;
    margin:5px;
   }
   .button_low{
      color: #000;
      background-color: #8c8c8c;
    }
   .state {
     font-size: 1.5rem;
     color:#8c8c8c;
     font-weight: bold;
   }
  </style>
<title>8266 物联网控制器</title>
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1>8266 物联网控制器</h1>
  </div>
  <div class="content">
    <div class="card">
      <h2>数字量引脚</h2>
      <p>
          <button id="d1" class="button button_small button_low">D1</button>
          <button id="d2" class="button button_small">D2</button>
          <button id="d3" class="button button_small">D3</button>
          <button id="d4" class="button button_small">D4</button>
          <button id="d5" class="button button_small">D5</button>
          <button id="d6" class="button button_small">D6</button>
          <button id="d7" class="button button_small">D7</button>
          <button id="d8" class="button button_small">D8</button>
        </p>
    </div>
    <div class="card">
      <h2>连接状态</h2>
      <p class="state"><span id="ws_status">%STATE%</span></p>
    </div>
  </div>
<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
    websocket.onerror = onError;
  }
  
  function onError(event){
    console.log("WebSocket Error:", event);
  }
  
  function onOpen(event) {
    console.log('Connection opened');
    document.getElementById('ws_status').innerHTML = "连接成功";
  }
  function onClose(event) {
    console.log('Connection closed');
    document.getElementById('ws_status').innerHTML = "连接关闭";
    //1秒后重联
    setTimeout(initWebSocket, 1000);
  }
  function onMessage(event) {
    console.log(event.data);
    /**
     * {d1:1,d2:1,d3:1,a0:10}
     **/
    var data = JSON.parse(event.data);
    console.log(data);
    for(pin in data){
        if(data[pin]==1){
            document.getElementById(pin).classList.remove("button_low");
        }else{
            document.getElementById(pin).classList.add("button_low");
        }
    }
  }
  
  function onLoad(event) {
    initWebSocket();
    initButton();
  }
  
  function initButton() {
    var obj = document.getElementsByTagName('button');
    for(var i=0; i<obj.length; i++){
      obj[i].addEventListener('click', btn_toggle);
    }
  }
  
  function btn_toggle(e){
    console.log(e);
    websocket.send(e.target.id);
  }
  
  var heart = setInterval(function(){
      websocket.send('heart');
      console.log('heart');
  }, 2000);
</script>
</body>
</html>
)rawliteral";
