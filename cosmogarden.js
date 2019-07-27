const codeField = document.querySelector('#code');
const logField = document.querySelector('#log');
const parseBtn = document.querySelector('#parse');
const runBtn = document.querySelector('#run');

let funcs = {};
let plantCallbacks = {
    branchGenerator: undefined,
};

const TDLParser = {
    // Array index 0: full match, 1: function name, 2: arguments, 3: function body
    functionRE: /function (\w+)\s*\(([\w,\s]*)\)\s*@([^@]*)@/g,
    compactSpaces: function (text) {
        return text.replace(/\s+/g, ' ');
    },

    extractArgs: function (argString) {
        return argString.split(',').map(s => s.trim());
    },

    parse: function (text) {
        let functions = {};

        text = this.compactSpaces(text);
        let fmatches = [...text.matchAll(this.functionRE)];

        for (let match of fmatches) {
            const funcName = match[1];
            const funcArgs = this.extractArgs(match[2]);
            const funcBody = match[3];
            try {
                functions[funcName] = new Function(...funcArgs, funcBody);
            } catch (err) {
                log(err + ' in function "' + funcName + '"');
            }
        }

        return functions;
    }
};

function log(text) {
    logField.value += '\n' + text;
    logField.scrollTop = logField.scrollHeight;
}

function parseTDL() {
    try {
        funcs = TDLParser.parse(codeField.value);
        log('Parse successful.')
    } catch (err) {
        log(err + ' in function "' + key + '" at line ' + err.lineNumber);
    }
}

function callFuncs() {
    if (!funcs.hasOwnProperty('main')) {
        log('No main function to run. Make sure to Parse first.');
        return;
    }

    try {
        funcs['main']();
    } catch(err) {
        log(err);
    }
}

function registerBranchGenerator(callback) {
    plantCallbacks.branchGenerator = function() {
        Module.setCPMemory(callback());
    };
    Module.setBranchCPGenerator(plantCallbacks.branchGenerator);
}

Module.onRuntimeInitialized = () => {
    Module.initLogger();
    codeField.value = '\/\/ All user functions will be stored in the \'funcs\' object\n\/\/ For example, funcs.exampleFunc\nfunction exampleFunc(a, b) @\n\treturn a + b;\n@\n\n\/\/ This is a sample function that generates X, Y, Z values for branch reference curves\n\/\/ We register this function in main() with registerBranchGenerator(funcs.genCPs);\n\/\/ Branch generation expects an array of 9 floats corresponding to the second, third,\n\/\/ and fourth control point of a cubic Bezier spline (the first point is 0, 0, 0)\nfunction genCPs() @\n\tvar cps = [];\n\tfor(var i = 0; i < 9; i++) {\n\t\tcps.push(Math.random() * 2);\n\t}\n\treturn cps; \n@\n\n\/\/ The code in \'main\' is run when you hit \'run\'\n\/\/ Built-in functions: log, registerBranchGenerator\nfunction main() @\n\tlog(\"The meaning of life is \" + funcs.exampleFunc(17, 25));\n\tregisterBranchGenerator(funcs.genCPs);\n@';
    logField.value = 'Welcome to CosmoGarden!';

    // Allow tab in textarea
    codeField.addEventListener('keydown', function (e) {
        if (e.code === 'Tab') {
            e.preventDefault();
            let s = this.selectionStart;
            this.value = this.value.substring(0, this.selectionStart) + "\t" + this.value.substring(this.selectionEnd);
            this.selectionEnd = s + 1;
        }
    });

    parseBtn.addEventListener('click', parseTDL);
    runBtn.addEventListener('click', callFuncs);
};