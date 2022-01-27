
let wsUri       = `ws://${location.hostname}:8081`
let websocket   = undefined
let zones       = {}                        // array item: #id : {name, html_div, counter}


let handleMessage   = (evt) => {
    let get_zone_id = (zone_id) => {
        return `${zone_id}-area-counter`
    }

    let jData       = JSON.parse (evt.data);


    if (jData["message-type"] == "scene-settings") {
        // Updating zones list
        let html_list   = $("#counters-list")

        for (zone_idx in jData["scene-settings"]) {
            let zone    = jData["scene-settings"][zone_idx]
            html_list.append (`<li class="nav-item pt-2" id="${get_zone_id(zone["zone_id"])}"></li>`)
            let item    = {
                name        : zone["zone_name"],
                html_div    : $(`#${get_zone_id(zone["zone_id"])}`),
                counter     : 0
            }
            item.html_div.text (`${item.name} area : ${item.counter}`)
            zones[zone["zone_id"]]  = item
        }
    }

    
    else if (jData["message-type"] == "detections") {
        // Updating detections statistics
        let detected_people = jData.data

        for (let i in zones) {
            zones[i].counter   = 0;
        }

        for (let i in detected_people) {
            let zone_id = detected_people[i].pos_zone.id
            zones[zone_id].counter++
        }

        for (let i in zones) {
            let item    = zones[i]
            item.html_div.text (`${item.name} area : ${item.counter}`)
        }
        document.getElementById ("img-el").src                          = `data:image/jpg;base64, ${jData.img}`
        $(`#global-counter`).text(`Detected People : ${detected_people.length}`)
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