const mqttBrokerUrl = 'wss://public.cloud.shiftr.io'; 
const mqttTopic = 'lq'; 
const clientId = 'Client-' + Math.random().toString(16).substr(2, 8); 
const username = 'public'; 
const password = 'public'; 

let client;
let savedBrightness = 125;
let savedSettings1 = {};
let savedSettings2 = {};

// 两组图片URL
const images = {
    ABDE: {
        left: "img/left.jpg",
        right: "img/right.jpg"
    },
    CF: {
        left: "img/lleft.jpg",
        right: "img/lright.jpg"
    }
};

// 假设当前选项ID存储在一个变量中，需要根据实际情况进行更新
let currentOptionId = 'optionA'; // 默认选项ID，需要根据用户的选择动态更新

const presets = {

    optionA: {
    //   ledBrightness: 125,
      servo1: { startAngle: { value: 0, min: 0, max: 90 }, endAngle: { value: 60, min: 0, max: 90 }, speed: {value: 500, min: 0, max: 1000}, times: {value: 1, min: 0, max: 20}, directionInterval: {value: 50, min: 0, max: 5000}, loopInterval: {value: 200, min: 0, max: 5000} },
      servo2: { startAngle: { value: 180, min: 90, max: 180 }, endAngle: { value: 120, min: 90, max: 180 }, speed: {value: 500, min: 0, max: 1000}, times: {value: 1, min: 0, max: 20}, directionInterval: {value: 50, min: 0, max: 5000}, loopInterval: {value: 200, min: 0, max: 5000} }
    }, //Tap
    optionB: {
        // ledBrightness: 185,
        servo1: { startAngle: { value: 0, min: 0, max: 90 }, endAngle: { value: 70, min: 0, max: 90 }, speed: {value: 500, min: 0, max: 1000}, times: {value: 2, min: 0, max: 20}, directionInterval: {value: 50, min: 0, max: 5000}, loopInterval: {value: 200, min: 0, max: 5000} },
        servo2: { startAngle: { value: 180, min: 90, max: 180 }, endAngle: { value: 110, min: 90, max: 180 }, speed: {value: 500, min: 0, max: 1000}, times: {value: 2, min: 0, max: 20}, directionInterval: {value: 50, min: 0, max: 5000}, loopInterval: {value: 200, min: 0, max: 5000} }
    }, //Squeeze
    optionC: {
        // ledBrightness: 25,
        servo1: { startAngle: { value: 180, min: 0, max: 180 }, endAngle: { value: 0, min: 0, max: 180 }, speed: {value: 600, min: 0, max: 1000}, times: {value: 3, min: 0, max: 20}, directionInterval: {value: 50, min: 0, max: 5000}, loopInterval: {value: 100, min: 0, max: 5000} },
        servo2: { startAngle: { value: 0, min: 0, max: 180 }, endAngle: { value: 180, min: 0, max: 180 }, speed: {value: 600, min: 0, max: 1000}, times: {value: 3, min: 0, max: 20}, directionInterval: {value: 50, min: 0, max: 5000}, loopInterval: {value: 100, min: 0, max: 5000} }
    }, //Rub
    optionD: {
        // ledBrightness: 105,
        servo1: { startAngle: { value: 40, min: 0, max: 90 }, endAngle: { value: 60, min: 0, max: 90 }, speed: {value: 1000, min: 0, max: 1000}, times: {value: 8, min: 0, max: 20}, directionInterval: {value: 50, min: 0, max: 5000}, loopInterval: {value: 50, min: 0, max: 5000} },
        servo2: { startAngle: { value: 130, min: 90, max: 180 }, endAngle: { value: 150, min: 90, max: 180 }, speed: {value: 1000, min: 0, max: 1000}, times: {value: 8, min: 0, max: 20}, directionInterval: {value: 50, min: 0, max: 5000}, loopInterval: {value: 50, min: 0, max: 5000} }
    }, //Tremble
    optionE: {
        // ledBrightness: 105,
        servo1: { startAngle: { value: 30, min: 0, max: 90 }, endAngle: { value: 60, min: 0, max: 90 }, speed: {value: 1000, min: 0, max: 1200}, times: {value: 5, min: 0, max: 20}, directionInterval: {value: 200, min: 0, max: 5000}, loopInterval: {value: 200, min: 0, max: 5000} },
        servo2: { startAngle: { value: 120, min: 90, max: 180 }, endAngle: { value: 150, min: 90, max: 180 }, speed: {value: 1000, min: 0, max: 1200}, times: {value: 5, min: 0, max: 20}, directionInterval: {value: 200, min: 0, max: 5000}, loopInterval: {value: 200, min: 0, max: 5000} }
    }, //Shake
    optionF: {
        // ledBrightness: 35,
        servo1: { startAngle: { value: 180, min: 0, max: 180 }, endAngle: { value: 140, min: 0, max: 180 }, speed: {value: 1000, min: 0, max: 1000}, times: {value: 10, min: 0, max: 20}, directionInterval: {value: 50, min: 0, max: 5000}, loopInterval: {value: 50, min: 0, max: 5000} },
        servo2: { startAngle: { value: 0, min: 0, max: 180 }, endAngle: { value: 40, min: 0, max: 180 }, speed: {value: 1000, min: 0, max: 1000}, times: {value: 10, min: 0, max: 20}, directionInterval: {value: 50, min: 0, max: 5000}, loopInterval: {value: 50, min: 0, max: 5000} }
    } //Vibration
  };

  let optionSettings = {
    optionA: null,
    optionB: null,
    optionC: null,
    optionD: null,
    optionE: null,
    optionF: null
  };
  

function updateValueDisplay(elementId) {
    var value = document.getElementById(elementId).value;
    document.getElementById(elementId + 'Display').textContent = value;
}


function updateSavedSettings(preset, optionId) {
    // save the setted brightness
    optionSettings[optionId] = {
        servo1: {...preset.servo1},
        servo2: {...preset.servo2}
    };

    // update UI
    updateServoUI('1', preset.servo1);
    updateServoUI('2', preset.servo2);
}


function updateServoUI(servoNumber, settings) {
    const startAngleInput = document.getElementById(`startAngle${servoNumber}`);
    startAngleInput.min = settings.startAngle.min;
    startAngleInput.max = settings.startAngle.max;
    startAngleInput.value = settings.startAngle.value;
    document.getElementById(`startAngle${servoNumber}Display`).textContent = settings.startAngle.value;

    // which servo it is and update the display values
    if (servoNumber === '1') {
        document.getElementById(`startAngle${servoNumber}Display`).textContent = settings.startAngle.value;
    } else if (servoNumber === '2') {
        document.getElementById(`startAngle${servoNumber}Display`).textContent = 180 - Number(settings.startAngle.value);
    }

    const endAngleInput = document.getElementById(`endAngle${servoNumber}`);
    endAngleInput.min = settings.endAngle.min;
    endAngleInput.max = settings.endAngle.max;
    endAngleInput.value = settings.endAngle.value;
    document.getElementById(`endAngle${servoNumber}Display`).textContent = settings.endAngle.value;

     // which servo it is and update the display values
     if (servoNumber === '1') {
        document.getElementById(`endAngle${servoNumber}Display`).textContent = settings.endAngle.value;
    } else if (servoNumber === '2') {
        document.getElementById(`endAngle${servoNumber}Display`).textContent = 180 - Number(settings.endAngle.value);
    }

    const speedInput = document.getElementById(`speed${servoNumber}`);
    speedInput.min = settings.speed.min;
    speedInput.max = settings.speed.max;
    speedInput.value = settings.speed.value;
    document.getElementById(`speed${servoNumber}Display`).textContent = settings.speed.value;

    const timesInput = document.getElementById(`times${servoNumber}`);
    timesInput.min = settings.times.min;
    timesInput.max = settings.times.max;
    timesInput.value = settings.times.value;
    document.getElementById(`times${servoNumber}Display`).textContent = settings.times.value;

    const directionIntervalInput = document.getElementById(`directionInterval${servoNumber}`);
    directionIntervalInput.min = settings.directionInterval.min;
    directionIntervalInput.max = settings.directionInterval.max;
    directionIntervalInput.value = settings.directionInterval.value;
    document.getElementById(`directionInterval${servoNumber}Display`).textContent = settings.directionInterval.value;

    const loopIntervalInput = document.getElementById(`loopInterval${servoNumber}`);
    loopIntervalInput.min = settings.loopInterval.min;
    loopIntervalInput.max = settings.loopInterval.max;
    loopIntervalInput.value = settings.loopInterval.value;
    document.getElementById(`loopInterval${servoNumber}Display`).textContent = settings.loopInterval.value;

}


  document.querySelectorAll('.preset-option').forEach(button => {
    button.addEventListener('click', function() {

// Get left panel 引用
var leftPanel = document.getElementById('leftPanel');

// hide all content
document.querySelectorAll('.panel-content').forEach(content => {
    content.classList.remove('active');
  });
    
// update left panel when clicking
if (['optionA', 'optionB', 'optionD', 'optionE'].includes(this.id)) {
  // if Option A, B, D, or E
  document.getElementById('contentABDE').classList.add('active');
  document.querySelector('.servo-image.left').src = images.ABDE.left; 
  document.querySelector('.servo-image.right').src = images.ABDE.right; 
} else if (['optionC', 'optionF'].includes(this.id)) {
  // if Option C or F
  document.getElementById('contentCF').classList.add('active');
  // update img
  document.querySelector('.servo-image.left').src = images.CF.left; 
  document.querySelector('.servo-image.right').src = images.CF.right; 
}

      // 移除其他按钮的选中状态
      document.querySelectorAll('.preset-option').forEach(btn => btn.classList.remove('selected'));
      
      // 为当前点击的按钮添加选中状态
      this.classList.add('selected');

     // 获取当前选项的ID
     const optionId = this.id;
     currentOptionId = optionId; // 更新当前选项ID

     // 检查是否有保存的设置，如果有，则使用保存的设置；如果没有，则使用预设值
     const settingsToUse = optionSettings[optionId] ? optionSettings[optionId] : presets[optionId];
    
    // 使用选中的设置更新UI
    updateUIWithSettings(settingsToUse);
    });
  });

  function updateUIWithSettings(settings) {

    updateServoUI('1', settings.servo1);
    updateServoUI('2', settings.servo2);
}

   
  function saveCurrentSettings(optionId) {
    // 获取当前UI中的设置
    const currentSettings = {
      servo1: {
        startAngle: document.getElementById('startAngle1').value,
        endAngle: document.getElementById('endAngle1').value,
        speed: document.getElementById('speed1').value,
        times: document.getElementById('times1').value,
        directionInterval: document.getElementById('directionInterval1').value,
        loopInterval: document.getElementById('loopInterval1').value,
        // 其他Servo 1设置...
      },
      servo2: {
        startAngle: document.getElementById('startAngle2').value,
        endAngle: document.getElementById('endAngle2').value,
        speed: document.getElementById('speed2').value,
        times: document.getElementById('times2').value,
        directionInterval: document.getElementById('directionInterval2').value,
        loopInterval: document.getElementById('loopInterval2').value,        
        // 其他Servo 2设置...
      }
    };
  
    // 更新optionSettings中对应选项的设置
    optionSettings[optionId] = currentSettings;
  }
  

document.getElementById('startAngle1').addEventListener('input', function() {
    document.getElementById('startAngle1Display').textContent = this.value;

     // 确保有为当前选项创建设置对象
     if (!optionSettings[currentOptionId]) {
        optionSettings[currentOptionId] = { ...presets[currentOptionId] }; // 如果没有，则从预设中复制
    }
    // 保存当前的startAngle设置到当前选项
    optionSettings[currentOptionId].servo1.startAngle.value = parseInt(this.value);
    
});

document.getElementById('endAngle1').addEventListener('input', function() {
    document.getElementById('endAngle1Display').textContent = this.value;
    // 确保有为当前选项创建设置对象
    if (!optionSettings[currentOptionId]) {
        optionSettings[currentOptionId] = { ...presets[currentOptionId] }; // 如果没有，则从预设中复制
    }
    // 保存当前的endAngle设置到当前选项
    optionSettings[currentOptionId].servo1.endAngle.value = parseInt(this.value);
});

document.getElementById('speed1').addEventListener('input', function() {
    document.getElementById('speed1Display').textContent = this.value;
    if (!optionSettings[currentOptionId]) {
        optionSettings[currentOptionId] = { ...presets[currentOptionId] }; 
    }
    // 保存当前的startAngle设置到当前选项
    optionSettings[currentOptionId].servo1.speed.value = parseInt(this.value);

     // 如果启MIRROR模式，则同时更新servo2
     if (document.getElementById('mirrorMode').checked) {
        // 更新servo2的UI和设置
        document.getElementById('speed2').value = this.value;
        document.getElementById('speed2Display').textContent = this.value;
        optionSettings[currentOptionId].servo2.speed.value = parseInt(this.value);
    }
});

document.getElementById('times1').addEventListener('input', function() {
    document.getElementById('times1Display').textContent = this.value;
    if (!optionSettings[currentOptionId]) {
        optionSettings[currentOptionId] = { ...presets[currentOptionId] }; 
    }
    // 保存当前的startAngle设置到当前选项
    optionSettings[currentOptionId].servo1.times.value = parseInt(this.value);

         // 如果启MIRROR模式，则同时更新servo2
     if (document.getElementById('mirrorMode').checked) {
        // 更新servo2的UI和设置
        document.getElementById('times2').value = this.value;
        document.getElementById('times2Display').textContent = this.value;
        optionSettings[currentOptionId].servo2.times.value = parseInt(this.value);
    }
});

document.getElementById('directionInterval1').addEventListener('input', function() {
    document.getElementById('directionInterval1Display').textContent = this.value;
    if (!optionSettings[currentOptionId]) {
        optionSettings[currentOptionId] = { ...presets[currentOptionId] }; 
    }
    // 保存当前的startAngle设置到当前选项
    optionSettings[currentOptionId].servo1.directionInterval.value = parseInt(this.value);

             // 如果启MIRROR模式，则同时更新servo2
             if (document.getElementById('mirrorMode').checked) {
                // 更新servo2的UI和设置
                document.getElementById('directionInterval2').value = this.value;
                document.getElementById('directionInterval2Display').textContent = this.value;
                optionSettings[currentOptionId].servo2.directionInterval.value = parseInt(this.value);
            }
});

document.getElementById('loopInterval1').addEventListener('input', function() {
    document.getElementById('loopInterval1Display').textContent = this.value;
    if (!optionSettings[currentOptionId]) {
        optionSettings[currentOptionId] = { ...presets[currentOptionId] }; 
    }
    // 保存当前的startAngle设置到当前选项
    optionSettings[currentOptionId].servo1.loopInterval.value = parseInt(this.value);

          // 如果启MIRROR模式，则同时更新servo2
          if (document.getElementById('mirrorMode').checked) {
            // 更新servo2的UI和设置
            document.getElementById('loopInterval2').value = this.value;
            document.getElementById('loopInterval2Display').textContent = this.value;
            optionSettings[currentOptionId].servo2.loopInterval.value = parseInt(this.value);
        }
});

// Repeat for servo 2 controls
document.getElementById('startAngle2').addEventListener('input', function() {
    document.getElementById('startAngle2Display').textContent = 180-Number(this.value);
         if (!optionSettings[currentOptionId]) {
            optionSettings[currentOptionId] = { ...presets[currentOptionId] }; 
        }
        // 保存当前的startAngle设置到当前选项
        optionSettings[currentOptionId].servo2.startAngle.value = parseInt(this.value);
});

document.getElementById('endAngle2').addEventListener('input', function() {
    document.getElementById('endAngle2Display').textContent =  180-Number(this.value);
    if (!optionSettings[currentOptionId]) {
        optionSettings[currentOptionId] = { ...presets[currentOptionId] }; 
    }
    // 保存当前的endAngle设置到当前选项
    optionSettings[currentOptionId].servo1.endAngle.value = parseInt(this.value);
});

document.getElementById('speed2').addEventListener('input', function() {
    document.getElementById('speed2Display').textContent = this.value;
    if (!optionSettings[currentOptionId]) {
        optionSettings[currentOptionId] = { ...presets[currentOptionId] }; 
    }
    // 保存当前的startAngle设置到当前选项
    optionSettings[currentOptionId].servo2.speed.value = parseInt(this.value);

         // 如果启用了镜像模式，则同时更新servo2的startAngle
         if (document.getElementById('mirrorMode').checked) {
            // 更新servo2的UI和设置
            document.getElementById('speed1').value = this.value;
            document.getElementById('speed1Display').textContent = this.value;
            optionSettings[currentOptionId].servo2.speed.value = parseInt(this.value);
         }
});

document.getElementById('times2').addEventListener('input', function() {
    document.getElementById('times2Display').textContent = this.value;
    if (!optionSettings[currentOptionId]) {
        optionSettings[currentOptionId] = { ...presets[currentOptionId] }; 
    }
    // 保存当前的startAngle设置到当前选项
    optionSettings[currentOptionId].servo2.times.value = parseInt(this.value);

             // 如果启MIRROR模式，则同时更新servo2
             if (document.getElementById('mirrorMode').checked) {
                // 更新servo2的UI和设置
                document.getElementById('times1').value = this.value;
                document.getElementById('times1Display').textContent = this.value;
                optionSettings[currentOptionId].servo2.times.value = parseInt(this.value);
            }
});

document.getElementById('directionInterval2').addEventListener('input', function() {
    document.getElementById('directionInterval2Display').textContent = this.value;
    if (!optionSettings[currentOptionId]) {
        optionSettings[currentOptionId] = { ...presets[currentOptionId] }; 
    }
    // 保存当前的startAngle设置到当前选项
    optionSettings[currentOptionId].servo2.directionInterval.value = parseInt(this.value);

                 // 如果启MIRROR模式，则同时更新servo2
                 if (document.getElementById('mirrorMode').checked) {
                    // 更新servo2的UI和设置
                    document.getElementById('directionInterval1').value = this.value;
                    document.getElementById('directionInterval1Display').textContent = this.value;
                    optionSettings[currentOptionId].servo2.directionInterval.value = parseInt(this.value);
                }
});

document.getElementById('loopInterval2').addEventListener('input', function() {
    document.getElementById('loopInterval2Display').textContent = this.value;
    if (!optionSettings[currentOptionId]) {
        optionSettings[currentOptionId] = { ...presets[currentOptionId] }; 
    }
    // 保存当前的startAngle设置到当前选项
    optionSettings[currentOptionId].servo2.loopInterval.value = parseInt(this.value);

      // 如果启MIRROR模式，则同时更新servo2
      if (document.getElementById('mirrorMode').checked) {
        // 更新servo2的UI和设置
        document.getElementById('loopInterval1').value = this.value;
        document.getElementById('loopInterval1Display').textContent = this.value;
        optionSettings[currentOptionId].servo2.loopInterval.value = parseInt(this.value);
    }
});



document.getElementById('saveSettings1').addEventListener('click', function() {
    savedSettings1 = {
        servoId: 0,
        startAngle: parseInt(document.getElementById('startAngle1').value),
        endAngle: parseInt(document.getElementById('endAngle1').value),
        speed: parseInt(document.getElementById('speed1').value), // 添加速度
        times: parseInt(document.getElementById('times1').value),
        directionInterval: parseInt(document.getElementById('directionInterval1').value),
        loopInterval: parseInt(document.getElementById('loopInterval1').value)
    };
    console.log("Settings saved for servo 1:", savedSettings1);
});

document.getElementById('generate1').addEventListener('click', function() {
    const savedSetting1 = [savedSettings1]
    sendMQTTMessage(JSON.stringify(savedSetting1));
});

document.getElementById('saveSettings2').addEventListener('click', function() {
    savedSettings2 = {
        servoId: 1,
        startAngle: parseInt(document.getElementById('startAngle2').value),
        endAngle: parseInt(document.getElementById('endAngle2').value),
        speed: parseInt(document.getElementById('speed2').value), // 添加速度
        times: parseInt(document.getElementById('times2').value),
        directionInterval: parseInt(document.getElementById('directionInterval2').value),
        loopInterval: parseInt(document.getElementById('loopInterval2').value)
    };
    console.log("Settings saved for servo 2:", savedSettings2);
});

document.getElementById('generate2').addEventListener('click', function() {
    const savedSetting2 = [savedSettings2]
    sendMQTTMessage(JSON.stringify(savedSetting2));
});

document.getElementById('saveSettingsBoth').addEventListener('click', function() {
    savedSettings1 = {
        servoId: 0,
        startAngle: document.getElementById('startAngle1').value,
        endAngle: document.getElementById('endAngle1').value,
        speed: parseInt(document.getElementById('speed1').value), // 添加速度
        times: document.getElementById('times1').value,
        directionInterval: parseInt(document.getElementById('directionInterval1').value),
        loopInterval: parseInt(document.getElementById('loopInterval1').value)
    };
    savedSettings2 = {
        servoId: 1,
        startAngle: document.getElementById('startAngle2').value,
        endAngle: document.getElementById('endAngle2').value,
        speed: parseInt(document.getElementById('speed2').value), // 添加速度
        times: document.getElementById('times2').value,
        directionInterval: parseInt(document.getElementById('directionInterval2').value),
        loopInterval: parseInt(document.getElementById('loopInterval2').value)
    };
    console.log("Settings saved for both servos:", savedSettings1, savedSettings2);
});

document.getElementById('generateBoth').addEventListener('click', function() {
    const settings1 = {
        servoId: 0,
        startAngle: parseInt(document.getElementById('startAngle1').value),
        endAngle: parseInt(document.getElementById('endAngle1').value),
        speed: parseInt(document.getElementById('speed1').value), // 添加速度
        times: parseInt(document.getElementById('times1').value),
        directionInterval: parseInt(document.getElementById('directionInterval1').value),
        loopInterval: parseInt(document.getElementById('loopInterval1').value)
    };

    const settings2 = {
        servoId: 1,
        startAngle: parseInt(document.getElementById('startAngle2').value),
        endAngle: parseInt(document.getElementById('endAngle2').value),
        speed: parseInt(document.getElementById('speed2').value), // 添加速度
        times: parseInt(document.getElementById('times2').value),
        directionInterval: parseInt(document.getElementById('directionInterval2').value),
        loopInterval: parseInt(document.getElementById('loopInterval2').value)
    };

    const combinedSettings = [[settings1, settings2]];
    sendMQTTMessage(JSON.stringify(combinedSettings));
});



function sendMQTTMessage(message) {
    if (client) {
        client.publish(mqttTopic, message);
        console.log("Message sent:", message);
    }
}

document.addEventListener('DOMContentLoaded', (event) => {
    client = mqtt.connect(mqttBrokerUrl, {
        clientId: clientId,
        username: username,
        password: password
    });

    client.on('connect', function () {
        console.log('Connected to MQTT Broker');
         // 订阅Arduino发送数据的主题
         client.subscribe(mqttTopic + "/servo1/status", function(err) {
            if (!err) {
              console.log("Subscription to '" + mqttTopic + "/servo1/status' successful");
            } else {
              console.error("Subscription error:", err);
            }
          });
    });

    client.on('error', function (error) {
        console.error('Connection error:', error);
    });

    // 监听MQTT消息
    client.on('message', function (topic, message) {
        // message是Buffer类型
        const msg = message.toString();
        console.log('Received message:', topic, msg);

        try {
            const data = JSON.parse(msg);
            if (topic === mqttTopic + "/servo1/status" && data.currentState === 1) {
              // 更新显示为1
              document.getElementById('currentStateDisplay').textContent = "1";
              
              // 设置一秒后将显示更新为0
              setTimeout(function() {
                document.getElementById('currentStateDisplay').textContent = "0";
              }, 1000); // 1000毫秒后执行
            }
          } catch (e) {
            console.error('Error parsing message:', e);
          }
    });

    // 确保初始预设被正确应用
    const initialSettings = optionSettings[currentOptionId] ? optionSettings[currentOptionId] : presets[currentOptionId];
    updateUIWithSettings(initialSettings);


    // Programmatically click the 'Option A' button to set it as default
    document.getElementById('optionA').click();
});


document.getElementById('recordBtn1').addEventListener('click', function() {
    const recordNameInput = document.getElementById('recordName1'); // 获取记录名称输入框
    const recordName = recordNameInput.value; // 获取输入的记录名称
    if (!recordName.trim()) {
        alert("Please enter a name for the settings.");
        return;
    }
    const settings1 = {
        name: recordName, // 保存名称
        servoId: 0,
        startAngle: parseInt(document.getElementById('startAngle1').value),
        endAngle: parseInt(document.getElementById('endAngle1').value),
        speed: parseInt(document.getElementById('speed1').value),
        times: parseInt(document.getElementById('times1').value),
        directionInterval: parseInt(document.getElementById('directionInterval1').value),
        loopInterval: parseInt(document.getElementById('loopInterval1').value)
    };
    saveSettingsToLocalStorage({servo1: settings1});
    //updateHistoryList({servo1: settings1}, null); // 根据需要更新UI

    recordNameInput.value = ''; // 清空输入框以便下一次输入
});

document.getElementById('recordBtn2').addEventListener('click', function() {
    const recordNameInput = document.getElementById('recordName2'); // 获取记录名称输入框
    const recordName = recordNameInput.value; // 获取输入的记录名称
    if (!recordName.trim()) {
        alert("Please enter a name for the settings.");
        return;
    }
    const settings2 = {
        name: recordName, // 保存名称
        servoId: 1,
        startAngle: parseInt(document.getElementById('startAngle2').value),
        endAngle: parseInt(document.getElementById('endAngle2').value),
        speed: parseInt(document.getElementById('speed2').value),
        times: parseInt(document.getElementById('times2').value),
        directionInterval: parseInt(document.getElementById('directionInterval2').value),
        loopInterval: parseInt(document.getElementById('loopInterval2').value)
    };
    saveSettingsToLocalStorage({servo2: settings2});
    //updateHistoryList({servo2: settings2}, null); // 根据需要更新UI
    recordNameInput.value = ''; // 清空输入框以便下一次输入
});

document.getElementById('recordBtnBoth').addEventListener('click', function() {
    const recordNameInput = document.getElementById('recordName'); // 获取记录名称输入框
    const recordName = recordNameInput.value; // 获取输入的记录名称
    if (!recordName.trim()) {
        alert("Please enter a name for the settings.");
        return;
    }
    const settings1 = {
        servoId: 0,
        startAngle: parseInt(document.getElementById('startAngle1').value),
        endAngle: parseInt(document.getElementById('endAngle1').value),
        speed: parseInt(document.getElementById('speed1').value),
        times: parseInt(document.getElementById('times1').value),
        directionInterval: parseInt(document.getElementById('directionInterval1').value),
        loopInterval: parseInt(document.getElementById('loopInterval1').value)
    };

    const settings2 = {
        servoId: 1,
        startAngle: parseInt(document.getElementById('startAngle2').value),
        endAngle: parseInt(document.getElementById('endAngle2').value),
        speed: parseInt(document.getElementById('speed2').value),
        times: parseInt(document.getElementById('times2').value),
        directionInterval: parseInt(document.getElementById('directionInterval2').value),
        loopInterval: parseInt(document.getElementById('loopInterval2').value)
    };

    saveSettingsToLocalStorage({name: recordName, servo1: settings1, servo2: settings2});
    //updateHistoryList({name: recordName, servo1: settings1, servo2: settings2}, null); // 根据需要更新UI
    recordNameInput.value = ''; // 清空输入框以便下一次输入
});











function saveSettingsToLocalStorage(settings) {
    let history = JSON.parse(localStorage.getItem('settingsHistory')) || [];
    history.push(settings);
    localStorage.setItem('settingsHistory', JSON.stringify(history));
    reloadHistoryList(); // 确保记录列表是最新的
}


function updateHistoryList(settings, index) {
    const historyList = document.getElementById('historyList');

    // 创建新的列表项
    const listItem = document.createElement('li');
    listItem.classList.add('record-item'); // 添加类名以便于 CSS 样式化

    // 构建列表项的内容
    const checkbox = document.createElement('input');
    checkbox.type = 'checkbox';
    checkbox.classList.add('recordCheckbox');
    if (index !== null) {
        checkbox.dataset.index = index;
    }

    const recordName = document.createElement('div');
    recordName.className = 'record-name';
    
        // 检查是否为单个 Servo 或整个记录命名
        if (settings.servo1 && settings.servo1.name) {
            recordName.innerHTML += `<strong>${settings.servo1.name}</strong><br>`;
        }
        if (settings.servo2 && settings.servo2.name) {
            recordName.innerHTML += `<strong>${settings.servo2.name}</strong><br>`;
        }
        if (settings.name) {
            recordName.innerHTML += `<strong>${settings.name}</strong><br>`; // 显示记录名称
        }

    const recordDetails = document.createElement('div');
    recordDetails.className = 'record-details';
    recordDetails.innerHTML = `${settings.servo1 ? `Left Tactor - Start Angle: ${settings.servo1.startAngle}, End Angle: ${settings.servo1.endAngle}, Speed: ${settings.servo1.speed}, Times: ${settings.servo1.times}, Direction Interval: ${settings.servo1.directionInterval}, Loop Interval: ${settings.servo1.loopInterval}<br>` : ''}
    ${settings.servo2 ? `Right Tactor - Start Angle: ${settings.servo2.startAngle}, End Angle: ${settings.servo2.endAngle}, Speed: ${settings.servo2.speed}, Times: ${settings.servo2.times}, Direction Interval: ${settings.servo2.directionInterval}, Loop Interval: ${settings.servo2.loopInterval}<br>` : ''}`;


    
    

    // const applyBtn = document.createElement('button');
    // applyBtn.className = 'applyBtn';
    // applyBtn.textContent = 'Apply';
    // // 绑定 apply 设置的逻辑


    // 创建Apply按钮
    const applyBtn = document.createElement('button');
    applyBtn.textContent = 'Apply';
    applyBtn.onclick = function() {
        applySettings(settings);
    };

    const deleteBtn = document.createElement('button');
    deleteBtn.className = 'deleteBtn';
    deleteBtn.textContent = 'Delete';
    // deleteBtn.addEventListener('click', function() {
    //     deleteRecord(index); // 直接使用闭包中的 index
    //     reloadHistoryList(); // 重新加载历史记录列表
    // });

    // 将构建的元素添加到列表项
    listItem.appendChild(checkbox);
    listItem.appendChild(recordName);
    listItem.appendChild(recordDetails);
    listItem.appendChild(applyBtn);
    listItem.appendChild(deleteBtn);

    // 将列表项添加到历史记录列表中
    historyList.appendChild(listItem);
}


document.getElementById('mergeAndApplyButton').addEventListener('click', function() {
    mergeAndApplySettings();
});


function mergeAndApplySettings() {
    const checkedBoxes = document.querySelectorAll('.recordCheckbox:checked');
    let mergedSettingsGroups = []; // 用于存储设置组的数组

    checkedBoxes.forEach(box => {
        const settingsIndex = parseInt(box.dataset.index, 10);
        const history = JSON.parse(localStorage.getItem('settingsHistory')) || [];
        const settingsGroup = history[settingsIndex]; // 获取一组设置

        // 如果两个伺服机都有设置，则作为一组添加
        if(settingsGroup && settingsGroup.servo1 && settingsGroup.servo2) {
            mergedSettingsGroups.push([settingsGroup.servo1, settingsGroup.servo2]);
        } else {
            // 否则，分别检查并添加单独的伺服机设置
            if(settingsGroup.servo1) {
                mergedSettingsGroups.push(settingsGroup.servo1);
            }
            if(settingsGroup.servo2) {
                mergedSettingsGroups.push(settingsGroup.servo2);
            }
        }
        
    });

    // 清理数组中的null值（如果有）
    mergedSettingsGroups = mergedSettingsGroups.filter(settings => settings !== null);

    if (mergedSettingsGroups.length > 1) {
        sendMQTTMessage(JSON.stringify(mergedSettingsGroups));
    } else {
        alert('No records selected!');
    }
}


document.addEventListener('DOMContentLoaded', function() {
    const historyList = document.getElementById('historyList');

// 尝试从localStorage获取历史记录
const history = JSON.parse(localStorage.getItem('settingsHistory')) || [];
// 对每个记录调用updateHistoryList来更新DOM
history.forEach((settings, index) => {
    updateHistoryList(settings, index);
});


    // 在historyList上设置事件监听器以处理所有的点击事件
    historyList.addEventListener('click', function(event) {
        // 检查是否点击了删除按钮
        if (event.target && event.target.nodeName === 'BUTTON' && event.target.textContent === 'Delete') {
            // 确定点击的按钮是列表中的第几个按钮
            const btn = event.target;
            const listItem = btn.parentNode;
            const index = Array.prototype.indexOf.call(historyList.children, listItem);

            deleteRecord(index);
            reloadHistoryList();
        }
    });
});


function deleteRecord(index) {
    // 从LocalStorage获取历史记录
    let history = JSON.parse(localStorage.getItem('settingsHistory')) || [];
    // 删除指定索引的记录
    history.splice(index, 1);
    // 保存更新后的历史记录到LocalStorage
    localStorage.setItem('settingsHistory', JSON.stringify(history));
    // 重新加载历史记录列表
    reloadHistoryList();
}

function reloadHistoryList() {
    // 清空当前的列表
    const list = document.getElementById('historyList');
    list.innerHTML = '';

    // 重新加载历史记录
    const history = JSON.parse(localStorage.getItem('settingsHistory')) || [];
    history.forEach((settings, index) => {
        // 注意这里调用的updateHistoryList函数已经适配了使用模板的方法
        updateHistoryList(settings, index);
    });
}

function addRecord(newSettings) {
    saveSettingsToLocalStorage(newSettings); // 保存设置到 localStorage
    // 不需要直接调用 updateHistoryList
    reloadHistoryList(); // 确保列表是最新的
}


function applySettings(settings) {
    let message;
    // 检查是单个伺服还是两个伺服的设置
    if (settings.servo1 && settings.servo2) {
        // 两个伺服
        message = JSON.stringify([[settings.servo1, settings.servo2]]);
    } else if (settings.servo1) {
        // 只有Servo 1
        message = JSON.stringify([settings.servo1]);
    } else if (settings.servo2) {
        // 只有Servo 2
        message = JSON.stringify([settings.servo2]);
    }

    // 发送消息
    if (message) {
        sendMQTTMessage(message);
    }
}



document.addEventListener('DOMContentLoaded', (event) => {
    // 初始化MQTT客户端
    client = mqtt.connect(mqttBrokerUrl, {
        clientId: clientId,
        username: username,
        password: password
    });

    // 连接到MQTT broker
    client.on('connect', function () {
        console.log('Connected to MQTT Broker');
        // 订阅Arduino发送消息的topic
        client.subscribe('lq/device/error', function(err) {
            if (!err) {
                console.log("Subscribed to 'lq/device/error'");
            }
        });
    });

    // 处理接收到的MQTT消息
    client.on('message', function (topic, message) {
        console.log('Received message:', topic, message.toString());
        // 检查topic是否匹配
        if (topic === 'lq/device/error') {
            const payload = JSON.parse(message.toString());
            // 根据消息内容更新网页
            if (payload.error === 1) {
                document.getElementById('deviceMessage').textContent = 'Fault';
            }
        }
    });

    // 按钮点击事件监听器
    document.getElementById('okButton').addEventListener('click', function() {
        // 当点击OK按钮时，重置设备消息显示为"等待设备消息..."
        document.getElementById('deviceMessage').textContent = 'Operational';
    });

});



