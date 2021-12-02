
let socket  = io();


socket.on ("new_data", (data) => {
    console.log (data)
    document.getElementById ("img-el").src  =`data:image/jpg;base64, ${data}`
})