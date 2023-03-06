/*
  Bare minimum client example for mqtt.js

  On document load, this script gets two divs from the HTML
  for local and remote messages. Then it attempts
  to connect to the broker. Once every two seconds, 
  it sends the local time if it's connected. 
  The publish button allows you to turn on and off publishing status.

  created 29 Dec 2022
  modified 5 Feb 2023
  by Tom Igoe
*/

// All these brokers work with this code. 
// Uncomment the one you want to use. 

////// emqx. Works in both basic WS and TLS WS:
// const broker = 'wss://broker.emqx.io:8084/mqtt'
// const broker = 'ws://broker.emqx.io:8083/mqtt'

//////// shiftr.io desktop client. 
// Fill in your desktop IP address for localhost:
// const broker = 'ws://localhost:1884';     

//////// shiftr.io, requires username and password 
// (see options variable below):
const broker = 'wss://public.cloud.shiftr.io';

//////// test.mosquitto.org, uses no username and password:
// const broker = 'wss://test.mosquitto.org:8081';

// MQTT client:
let client;

// connection options:
let options = {
  // Clean session
  clean: true,
  // connect timeout in ms:
  connectTimeout: 10000,
  // Authentication
  clientId: 'mqttJsClient',
  // add these in for public.cloud.shiftr.io:
  username: 'public',
  password: 'public'
}
// topic to subscribe to when you connect:
let topic = 'conndev/lq2078/#';
let topicUp = 'conndev/lq2078/dis';
let topicW1 = 'conndev/lq2078/warn';
// divs to show messages:
let localDiv, remoteDiv, remoteDivT, remoteDiv1, remoteDiv1T, remoteDiv2;
// whether the client should be publishing or not:
let publishing = true;

function setup() {
  // put the divs in variables for ease of use:
  localDiv = document.getElementById('local');
  remoteDiv = document.getElementById('remote');
  remoteDivT = document.getElementById('remoteT');
  remoteDiv1 = document.getElementById('remote1');
  remoteDiv1T = document.getElementById('remote1T');
  remoteDiv2 = document.getElementById('remote2');

  // set text of localDiv:
  localDiv.innerHTML = 'trying to connect';
  // attempt to connect:
  client = mqtt.connect(broker, options);
  // set listeners:
  client.on('connect', onConnect);
  client.on('close', onDisconnect);
  client.on('message', onMessage);
  client.on('error', onError);
}


// handler for mqtt connect event:
function onConnect() {
  // update localDiv text:
  localDiv.innerHTML = 'connected to broker. Subscribing...'
  // subscribe to the topic:
  client.subscribe(topic, onSubscribe);
}

// handler for mqtt disconnect event:
function onDisconnect() {
  // update localDiv text:
  localDiv.innerHTML = 'disconnected from broker.'
}

// handler for mqtt error event:
function onError(error) {
  // update localDiv text:
  localDiv.innerHTML = error;
}

// handler for mqtt subscribe event:
function onSubscribe(response, error) {
  if (!error) {
    // update localDiv text:
    localDiv.innerHTML = 'Subscribed to broker.';
  } else {
    // update localDiv text with the error:
    localDiv.innerHTML = error;
  }
}

// handler for mqtt message received event:
function onMessage(topic, payload, packet) {
  //let result = 'received a message on topic:  ' + topic;
  // message is  a Buffer, so convert to a string:
  let result = payload.toString();

 if(topic == topicUp){
  remoteDivT.innerHTML = result;
  remoteDiv.style.opacity= 300/payload.toString();
  remoteDiv.style.width = payload.toString()*0.5+"px";
  remoteDiv.style.height = payload.toString()*0.5+"px";
  if (result>=1000 ){
    remoteDivT.innerHTML = 'Leave!!!';
    remoteDiv.style.opacity= 1;
  remoteDiv.style.width = "450px";
  remoteDiv.style.height = "450px";
  }
 }

 if(topic == topicW1){
  // remoteDiv1.innerHTML = result;
  remoteDiv1T.innerHTML = result;
  remoteDiv1.style.width = 20+payload.toString()*1+"px";
  remoteDiv1.style.height = 20+payload.toString()*1+"px";
 }
}

// on page load, call the setup function:
document.addEventListener('DOMContentLoaded', setup);
// run a loop every 2 seconds:
setInterval(loop, 2000);