const { SerialPort } = require('serialport')
const { ReadlineParser } = require('@serialport/parser-readline')
const colors = require('colors/safe');

const port = new SerialPort({ path: '/dev/tty.usbserial-A90FBD9D', baudRate: 115200 })
const sleep = ms => new Promise(r => setTimeout(r, ms));
const cmdRegex = /Uart cli handle command => '(.*)'/;
const commands = ['12345','98765','HelloWorld','GoodBye'];
const receivedCommands = [];

const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }))
parser.on('data', (data) => {
    data = data.trim();
    console.log(colors.gray("uart>", data));
    const matches = cmdRegex.exec(data);
    if(matches){
        const cmd = matches[1];
        console.log(colors.cyan("recv>", cmd));
        receivedCommands.push(cmd);    
    }
});
const send = cmd => {
    console.log(colors.magenta('send>', cmd));
    port.write(cmd+"\n");    
};


const run = async function() {
    console.log("** Send first two commands");
    send(commands[0]);
    await sleep(10);
    send(commands[1]);
    await sleep(500);

    console.log("** Wait until asleep again");
    await sleep(10*1000);

    console.log("** Send second two commands");
    send(commands[2]);
    await sleep(10);
    send(commands[3]);
    await sleep(500);

    // console.log("** Commands received => ", receivedCommands);
    // console.log("** Commands sent     => ", commands);

    console.log();
    console.log();

    for (let i = 0; i < commands.length; i++) {
        if(receivedCommands[i] !== commands[i]) {
            console.log(colors.red(`Missmatch on ${i}:`));
            console.log(`  send: `, colors.magenta(`${commands[i]}`));
            console.log(`  recv: `, colors.cyan(`${receivedCommands[i]}`));
        }   
    }

    port.close();
}

run();
