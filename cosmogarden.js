const codeField = document.querySelector('#code');
const logField = document.querySelector('#log');
const parseBtn = document.querySelector('#parse');
const runBtn = document.querySelector('#run');

let builtins = {};
let funcs = {};
let plantCallbacks = {};

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
        Module.buildTree();
    } catch (err) {
        log(err);
    }
}

function register(builtinName, callback) {
    const builtin = builtins[builtinName];

    if (!builtin) {
        log("No builtin called '" + builtinName + "' exists.");
        return;
    }

    if (!callback) {
        log("Trying to register invalid function to builtin '" + builtinName + ".'");
        return;
    }

    registerPlantCallback(name, callback, builtin.set, builtin.gen);
}

// name (string): the name of the function (arbitrary but unique)
// callback (function): the JavaScript function that returns the desired value (user defined)
// setFunc (function): the C++ function that updates to the result of calling callback(), eg Module.setNumBranches
// genFunc (function): the C++ function that hooks up the tree generator with the plant callback, eg Module.setNumBranchesFunc
function registerPlantCallback(name, callback, setFunc, genFunc) {
    plantCallbacks[name] = function () {
        setFunc(callback());
    };
    genFunc(plantCallbacks[name]);
}

Module.onRuntimeInitialized = () => {
    Module.initLogger();
    codeField.value =  '\/\/ Branch curves are an array of 9 floats corresponding to the second, third,\n\/\/ and fourth control point of a cubic Bezier spline (the first point is 0, 0, 0)\nfunction genCurve() @\n\tvar cps = [];\n\tfor(var i = 0; i < 9; i++) {\n\t\tcps.push(Math.random() * 2);\n\t}\n\treturn cps; \n@\n\n\/\/ timesToBranch is per tree and not per branch because...\n\/\/ when would it stop branching? Needs design if it\'s to vary\n\/\/ Expects an int\nfunction timesToBranch() @\n\treturn 3;\n@\n\n\/\/ When a branch splits, it splits into this many branches\n\/\/ Expects an int - floats are implicitly floored, eg 3.99 -> 3\nfunction numBranches() @\n\treturn Math.random() * 3 + 2;\n@\n\n\/\/ The code in \'main\' is run when you hit \'run\'\n\/\/ Built-in functions: log, register\n\/\/ Valid values for register\'s first parameter:\n\/\/ \"branchCurve\" \"timesToBranch\" \"numBranches\"\nfunction main() @\n\tlog(\"Building tree...\");\n\tregister(\"branchCurve\", funcs.genCurve);\n\tregister(\"timesToBranch\", funcs.timesToBranch);\n\tregister(\"numBranches\", funcs.numBranches);\n@';
    logField.value = 'Welcome to CosmoGarden!';


    builtins = {
        "branchCurve": {
            set: Module.setBranchCurve,
            gen: Module.setBranchCurveFunc
        },
        "timesToBranch" : {
            set: Module.setTimesToBranch,
            gen: Module.setTimesToBranchFunc
        },
        "numBranches" : {
            set: Module.setNumBranches,
            gen: Module.setNumBranchesFunc
        },
    };

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