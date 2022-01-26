
let wsUri       = "ws://localhost:8081"
let websocket   = undefined


let handleMessage   = (evt) => {
    //console.log (evt.data);
    let jData   = JSON.parse (evt.data);
    //console.log (jData.data)

    if (jData["message-type"] == "scene-settings") {
        // TODO Updating scene-settings
    }
    else if (jData["message-type"] == "detections") {
        // FIXME Handle possibility of having many zones
        // Updating detections statistics
        dp              = jData.data
        b_area_count    = 0
        c_area_count    = 0
        d_area_count    = 0
        w_area_count    = 0
        for (let i in dp) {
            let d   = dp[i]
            console.log (d)
            switch (d.pos_zone.id) {
                case 0:
                    d_area_count++;
                    break;
                case 1:
                    c_area_count++;
                    break;
                case 2:
                    b_area_count++;
                    break;
                case 3:
                    w_area_count++;
                    break;
                default:
                    console.error (`Undefined zone id: ${d.pos_zone.id}`)
            }
        }
        document.getElementById ("img-el").src                          = `data:image/jpg;base64, ${jData.img}`
        document.getElementById ("blackboard-area-counter").innerHTML   = `Blackboard area : ${b_area_count}`
        document.getElementById ("coffe-area-counter").innerHTML        = `Coffe area : ${c_area_count}`
        document.getElementById ("desk-area-counter").innerHTML         = `Desk area : ${d_area_count}`
        document.getElementById ("window-area-counter").innerHTML       = `Window area : ${w_area_count}`
    }
}


let initWebSocket   = () => {
    try {
        if (typeof MozWebSocket == 'function')
            WebSocket = MozWebSocket;
        if ( websocket && websocket.readyState == 1 )
            websocket.close();
        
        websocket = new WebSocket( wsUri );
        websocket.onmessage = function (evt) {
            handleMessage (evt)
        };
        websocket.onerror = function (evt) {
            console.log ('ERROR: ' + evt.data);
        };
    } catch (exception) {
        console.log ('ERROR: ' + exception);
    }
}

initWebSocket()