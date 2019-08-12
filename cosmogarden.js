const codeField = document.querySelector('#code');
const logField = document.querySelector('#log');
const parseBtn = document.querySelector('#parse');
const runBtn = document.querySelector('#run');

let funcs = {};
let branchScalars = [];
let branchFactory = {
    genBranchScaleFunc: () => { return () => 1.0; },
    genFurcations: (depth, furcation, branch) => depth === 0 ? [0.2, 0.6] : [],
    genNumBranches: (depth, furcation, branch) => 3,
    genCurve: (depth, furcation, branch) => [0.0, 0.0, 0.0, 3.0, -3.0, 1.0, 0.1, 1.0, 0.0, 2.0, 0.0, 1.1],
    genScale: (depth, furcation, branch) => Math.pow(0.6, depth),
    genAngle: (depth, furcation, branch) => depth === 0 ? 0.0 : 120.0 * branch,
    genTicksToGrow: (depth, furcation, branch) => 50,
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
        branchScalars = [];
        let trunk = furcate(0, 0, 0, 0.0);
        Module.buildPlant(trunk);
    } catch (err) {
        log("Run error: " + err);
    }
}

function getBranchScale(id, growthRatio, distAlongCurve) {
    return branchScalars[id](growthRatio, distAlongCurve);
}

Module.onRuntimeInitialized = () => {
    Module.initAll();
    // Converted using http://www.howtocreate.co.uk/tutorials/jsexamples/syntax/prepareInline.html
    codeField.value = '\/\/ function(float growthRatio, float distAlongCurve) -> float branchScalar\n\/\/ Returns a function that takes two floats and returns a scalar.\n\/\/ The branch\'s cross section will be scaled by this scalar\nfunction genBranchScaleFunc(depth, furcation, branch) @\n\tif(depth === 0) {\n\t\treturn (growthRatio, distAlongCurve) => 0.25 * growthRatio * Math.pow(1.0 - distAlongCurve, 0.4);\n\t}\n\n\treturn function(growthRatio, distAlongCurve) {\n\t\treturn 0.1 * growthRatio * Math.pow(1.0 - distAlongCurve, 0.3);\n\t};\n@\n\n\/\/ array[n floats]\n\/\/ A list of float distances along the branch that\n\/\/ furcations occur at (between 0 and 1)\nfunction genFurcations(depth, furcation, branch) @\n\tif(depth === 0) return [0.2, 0.7];\n\tif(depth === 1) return [0.5];\n\treturn [];\n@\n\n\/\/ integer\n\/\/ The number of branches in a furcation\nfunction genNumBranches(depth, furcation, branch) @\n\tif(furcation === 0) return 3; \n\treturn 2;\n@\n\n\/\/ array[12 floats]\n\/\/ Branch curves are an array of 12 floats corresponding \n\/\/ to the X, Y, Z values of the 4 control points of a \n\/\/ quadratic bezier curve\nfunction genCurve(depth, furcation, branch) @\n\tif(depth === 0) return [0.0, -2.0, 0.0, -0.5, 0.0, 0.5, 0.5, 2.0, 0.8, -2.0, 3.0, 1.8];\n\n\tvar cps = [0.0, 0.0, 0.0];\n\tfor(var i = 0; i < 9; i++) {\n\t\tcps.push(Math.random() * 3);\n\t}\n\treturn cps; \n@\n\n\/\/ float\n\/\/ How big is the branch curve?\nfunction genScale(depth, furcation, branch) @\n\treturn Math.pow(0.62, depth);\n@\n\n\/\/ float\n\/\/ What is the angle, in degrees, of roll around the parent branch?\nfunction genAngle(depth, furcation, branch) @\n\treturn branch * 120.0;\n@\n\n\/\/ int\n\/\/ How many ticks will it take to grow the branch once it begins?\nfunction genTicksToGrow(depth, furcation, branch) @\n\treturn 50 + depth * 20 + branch * 10;\n@\n\n\/\/ The code in \'main\' is run when you hit \'run\'\nfunction main() @\n\tlog(\"Building tree...\");\n\tbranchFactory.genBranchScaleFunc = funcs.genBranchScaleFunc;\n\tbranchFactory.genFurcations = funcs.genFurcations;\n\tbranchFactory.genNumBranches = funcs.genNumBranches;\n\tbranchFactory.genCurve = funcs.genCurve;\n\tbranchFactory.genScale = funcs.genScale;\n\tbranchFactory.genAngle = funcs.genAngle;\n\tbranchFactory.genTicksToGrow = funcs.genTicksToGrow;\n@';
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

function branchDescription(depth, furcation, branch, startRatio) {
    let bd = {};
    bd.id = branchScalars.length;
    branchScalars.push(branchFactory.genBranchScaleFunc(depth, furcation, branch));
    bd.startRatio = startRatio;
    bd.curve = branchFactory.genCurve(depth, furcation, branch);
    bd.scale = branchFactory.genScale(depth, furcation, branch);
    bd.angle = branchFactory.genAngle(depth, furcation, branch);
    bd.ticksToGrow = branchFactory.genTicksToGrow(depth, furcation, branch);
    bd.children = Module.makeBranchVector();
    return bd;
}

function furcate(depth, furcation, branch, startRatio) {
    let newBranch = branchDescription(depth, furcation, branch, startRatio);
    let furcs = branchFactory.genFurcations(depth, furcation, branch);

    for(let furcNum = 0; furcNum < furcs.length; furcNum++) {
        let numBranches = branchFactory.genNumBranches(depth, furcNum, branch);
        let furcStartRatio = furcs[furcNum];

        for(let branchNum = 0; branchNum < numBranches; branchNum++) {
            newBranch.children.push_back(furcate(depth + 1, furcNum, branchNum, furcStartRatio));
        }
    }

    return newBranch;
}