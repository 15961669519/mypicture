/**
 * 将一个大字符串（<14kb）保存到闪存中
 * 
 * 注意：%是特殊字符串，用于变量替换
 *      比如：%state% 可以用 processor 函数进行替换
 *      如果同一行代码需要出现两次%，比如 size:100% 100%; 必须改为 size:100%% 100%%;
 *      
 *      %heart_time% 是我们设定的心跳频率
 */
const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE HTML>
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

        .data-list {
            display: flex;

        }

        .data {
            display: flex;
            border: solid 1px #000;
            background-color: #143642;
            border-radius: 8px;
            color: #fff;
            margin: 5px;
            flex: 1 1 auto;
            justify-content: center;
        }

        .data-info {
            display: flex;
            flex-flow: column;
            padding-right: 5px;
        }

        .data-info .title {
            height: 30px;
            display: flex;
            align-items: center;
            color: #F8F7F9;
        }

        .data-info .value {
            height: 30px;
            display: flex;
            justify-content: center;
            align-items: center;
            font-size: 18px;
            font-weight: bold;
        }

        .data .icon {
            background-size: 100%;
            width: 60px;
            height: 60px;
            display: flex;
        }

        .icon_default {
            background: url() left center;
        }

        .temperature {
            background: url("data:image/png;base64,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") left center;
        }

        .humidity {
            background: url("data:image/png;base64,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") left center;
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
            <h2>sht30传感器，使用D3，D4引脚</h2>
            <div class="data-list">
                <div class="data">
                    <div class="icon temperature"></div>
                    <div class="data-info">
                        <span class="value" id="temperature">temperation</span>
                        <span class="title">温度</span>
                    </div>
                </div>
                <div class="data">
                    <div class="icon humidity"></div>
                    <div class="data-info">
                        <span class="value"><span id="humidity">humidity</span>%</span>

                        <span class="title">湿度</span>
                    </div>
                </div>
            </div>
        </div>

        <div class="card">
            <h2>通用传感器</h2>
            <div class="data-list">
                <div class="data">
                    <div class="data-info">
                        <span class="value" id="d1">-</span>
                        <span class="title">D1</span>
                    </div>
                </div>
                <div class="data">
                    <div class="data-info">
                        <span class="value" id="d2">-</span>

                        <span class="title">D2</span>
                    </div>
                </div>
                <div class="data">
                    <div class="data-info">
                        <span class="value" id="a0">1023</span>

                        <span class="title">模拟量(A0)</span>
                    </div>
                </div>
            </div>
        </div>

        <div class="card">
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
                <span id="ws_status">STATE</span>
                (编号 <span id="client_id">CLIENT_ID</span>)
                <a style="color:#0f8b8d" href="#" onclick="javascript:location.reload();">(刷新)</a>
            </p>
        </div>
    </div>
    <script>
        var gateway = `ws://${window.location.hostname}/ws`;
        var heart_timeout = %heart_timeout%;
        var heart_last_time = 0;
//        var gateway = `ws://192.168.1.204/ws`;
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
            if (data.hasOwnProperty("client_id")) {
                client_id = data['client_id'];
                document.querySelector("#client_id").innerHTML = client_id;
            }

            for (pin in data) {
                let id_object = document.querySelector("#" + pin);

                if (id_object) {
                    if (id_object.tagName == 'SPAN') {
                        id_object.innerHTML = data[pin];
                    }
                    if (id_object.tagName == 'INPUT') {
                        id_object.value = data[pin];
                    }
                }
            }
        }

        function onLoad(event) {
            initWebSocket();
        }

        var heart = setInterval(function () {
            websocket.send(JSON.stringify({ "type": "heart", "client_id": client_id }));
            console.log('heart');

            //检查距离上次收到消息的时间，判断设备在线状态
            var ts = new Date().getTime();
            if (heart_last_time > 0) {
                if (ts - heart_last_time > heart_timeout * 1.5) {
                    console.info("连接丢失，已经断开连接");
                    document.querySelector('#ws_status').innerHTML = "连接断开";

                }
            }
        }, heart_timeout);


        //一旦pwm数值发生变化，则发给服务端
        var slideChange = (id, value) => {
            console.info("=====", id, value);
            websocket.send(JSON.stringify({ "type": "pwm", "id": id, "value": value }));
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
