const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>

<head>
    <title>8266物联网控制器（1024编程实验室）</title>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
    <meta name="viewport"
        content="width=device-width,initial-scale=1,minimum-scale=1,maximum-scale=1,user-scalable=no" />
    <link rel="icon" href="data:,">
    <style>
        html {
            font-family: Arial, Helvetica, sans-serif;
            text-align: center;
        }

        h1 {
            font-size: 1.5rem;
            color: white;
        }

        h2 {
            font-size: 1.2rem;
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
            padding: 10px;
            max-width: 600px;
            margin: 0 auto;
        }

        .card {
            background-color: #F8F7F9;
            ;
            box-shadow: 2px 2px 12px 1px rgba(140, 140, 140, .5);
            padding-top: 10px;
            padding-bottom: 20px;
        }

        .button {
            padding: 15px 20px;
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
            -webkit-tap-highlight-color: rgba(0, 0, 0, 0);
        }

        /*.button:hover {background-color: #0f8b8d}*/
        .button:active {
            /* background-color: #0f8b8d; */
            box-shadow: 2 2px #CDCDCD;
            transform: translateY(2px);
        }

        .button_small {
            padding: 10px;
            margin: 3px;
        }

        .button_low {
            color: #000;
            background-color: #8c8c8c;
        }

        .state {
            font-size: 1.5rem;
            color: #8c8c8c;
            font-weight: bold;
        }

        .pwm-card {
            display: flex;
        }

        .pwm-card button {
            width: 80px;
        }

        .slide {
            width: 100%;
        }
    </style>
    <title>8266 物联网控制器</title>
    <link rel="icon" href="data:,">
</head>

<body>
    <script>

    </script>
    <div class="topnav">
        <h1>8266 物联网控制器 PWM</h1>
    </div>
    <div class="content">
        <div class="card">
            <div class="pwm-card">
                <button id="d1-min" class="button button_small button_low">D1</button>
                <input type="range" id="d1" min="0" max="255" value="0" step="15" class="slide">
                <button id="d1-max" class="button button_small">D1</button>
            </div>
            <div class="pwm-card">
                <button id="d2-min" class="button button_small button_low">D2</button>
                <input type="range" id="d2" min="0" max="255" value="0" step="15" class="slide">
                <button id="d2-max" class="button button_small">D2</button>
            </div>
            <div class="pwm-card">
                <button id="d3-min" class="button button_small button_low">D3</button>
                <input type="range" id="d3" min="0" max="255" value="0" step="15" class="slide">
                <button id="d3-max" class="button button_small">D3</button>
            </div>
            <div class="pwm-card">
                <button id="d4-min" class="button button_small button_low">D4</button>
                <input type="range" id="d4" min="0" max="255" value="0" step="15" class="slide">
                <button id="d4-max" class="button button_small">D4</button>
            </div>
            <div class="pwm-card">
                <button id="d5-min" class="button button_small button_low">D5</button>
                <input type="range" id="d5" min="0" max="255" value="0" step="15" class="slide">
                <button id="d5-max" class="button button_small">D5</button>
            </div>
            <div class="pwm-card">
                <button id="d6-min" class="button button_small button_low">D6</button>
                <input type="range" id="d6" min="0" max="255" value="0" step="15" class="slide">
                <button id="d6-max" class="button button_small">D6</button>
            </div>
            <div class="pwm-card">
                <button id="d7-min" class="button button_small button_low">D7</button>
                <input type="range" id="d7" min="0" max="255" value="0" step="15" class="slide">
                <button id="d7-max" class="button button_small">D7</button>
            </div>
            <div class="pwm-card">
                <button id="d8-min" class="button button_small button_low">D8</button>
                <input type="range" id="d8" min="0" max="255" value="0" step="15" class="slide">
                <button id="d8-max" class="button button_small">D8</button>
            </div>
        </div>
        <div class="card">
            <p class="state">
                <span id="ws_status">%STATE%</span>
                (编号 <span id="client_id">%CLIENT_ID%</span>)
                <a style="color:#0f8b8d" href="#" onclick="javascript:location.reload();">(刷新)</a>
            </p>
        </div>
    </div>
    <script>
        var gateway = `ws://${window.location.hostname}/ws`;
        var heart_timeout = 3000;
        var heart_last_time = 0;
        // var gateway = `ws://192.168.1.204/ws`;
        var websocket;
        var client_id = 0;
        window.addEventListener('load', onLoad);
        function initWebSocket() {
            console.log('Trying to open a WebSocket connection...');
            websocket = new WebSocket(gateway);
            websocket.onopen = onOpen;
            websocket.onclose = onClose;
            websocket.onmessage = onMessage;
            websocket.onerror = onError;
        }

        function onError(event) {
            console.log("WebSocket Error:", event);
        }

        function onOpen(event) {
            console.log('Connection opened');
            document.querySelector('#ws_status').innerHTML = "连接成功";
        }
        function onClose(event) {
            console.log('Connection closed');
            document.querySelector('#ws_status').innerHTML = "连接关闭";
            //1秒后重联
            setTimeout(initWebSocket, 1000);
        }
        //收到消息后
        function onMessage(event) {
            //更新收到消息的时间
            heart_last_time = new Date().getTime();

            console.log(event.data);

            var data = JSON.parse(event.data);
            console.log(data);

            //如果在消息中发现了 client_id 则说明是首次连接发来的消息
            if( data.hasOwnProperty("client_id") ){
                client_id = data['client_id'];
                document.querySelector("#client_id").innerHTML = client_id;
            }

            for (pin in data) {
                var id_object = document.querySelector("#"+pin);
                if(id_object){
                    id_object.value = data[pin];
                }
            }
        }

        function onLoad(event) {
            initWebSocket();
        }

        var heart = setInterval(function () {
            websocket.send(JSON.stringify({"type":"heart","client_id":client_id}));
            console.log('heart');

            //检查距离上次收到消息的时间，判断设备在线状态
            var ts = new Date().getTime();
            if(heart_last_time>0){
                if( ts-heart_last_time>heart_timeout*1.5 ){
                    console.info("连接丢失，已经断开连接");
                    document.querySelector('#ws_status').innerHTML = "连接断开";
                    
                }
            }            
        }, heart_timeout);


        //一旦pwm数值发生变化，则发给服务端
        var slideChange = (id, value) => {
            console.info("=====",id,value);
            websocket.send(JSON.stringify({"type":"pwm","id":id, "value":value}));
        }

        //绑定max 和 min 的按钮
        var pwm_card = document.querySelectorAll(".pwm-card");
        for (var i = 0; i < pwm_card.length; i++) {
            var button = pwm_card[i].querySelectorAll("button");
            for (var n = 0; n < button.length; n++) {
                button[n].addEventListener('click', (e) => {
                    var data = (e.target.id).split("-")
                    var value = data[1] == 'max' ? 255 : 0;
                    slideChange(data[0], value);
                    document.getElementById(data[0]).value = value;

                })
            }
        }
        //绑定slide数值发生变化的事件
        var slideData = document.querySelectorAll(".slide");
        for (var i = 0; i < slideData.length; i++) {
            slideData[i].onchange = (e) => {
                slideChange(e.target.id, e.target.value);
            }
        }
    </script>
</body>

</html>
)rawliteral";
