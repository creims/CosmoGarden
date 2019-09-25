const codeField = document.querySelector('#code');
const logField = document.querySelector('#log');
const cmdField = document.querySelector('#cmd');
const parseBtn = document.querySelector('#parse');
const runBtn = document.querySelector('#run');

const TAB = '   ';

let plantGen;
let currentPlant;
let branchScalers = {
    scalers: {},
    numRegistered: 0,
    getBranchScale: function(id, growthRatio, distAlongCurve) {
        return this.scalers[id](growthRatio, distAlongCurve);
    },
    registerScaler: function(fn) {
        this.scalers[this.numRegistered] = fn;
        return this.numRegistered++;
    },
    clear: function() {
        this.scalers = {};
        this.numRegistered = 0;
    }
};

function getBranchScale(id, g, d) {
    return branchScalers.getBranchScale(id, g, d);
}

function log(text) {
    logField.value += '\n' + text;
    logField.scrollTop = logField.scrollHeight;
}

function handleParse() {
    try {
        plantGen = compileCOSML(codeField.value);
        log('Parse successful.');
    } catch (e) {
        log(e);
        plantGen = undefined;
    }
}

function handleRun() {
    if(!plantGen) {
        log('No valid plant parsed.');
    } else {
        const plant = plantGen.build();
        branchScalers.clear();
        currentPlant = {};
        currentPlant.trunks = makeVector(Module.makeBranchVector, plant.map(child =>
                registerBranch(child)
            ));

        Module.buildPlant(currentPlant);
    }
}

function isReserved(name) {
    if(name === 'default' || name === 'lastsession') {
        return true;
    }

    return false;
}

function loadCode(name) {
    if(name === 'default') {
        codeField.value = '# This default program can be recovered via the "load default" command\n# \'#\' specifies comments\n# The plant keyword needs to be defined once, and specifies the plant\nplant {\n   # Specifying a non-0 start ratio at depth 0 is a bad idea\n   Branch_A\n   {\n      0.7: Branch_A\n      {\n         0.7: Branch_B{}*2\n         .6~.8: Branch_B{angle=30~190} # The range operator \'~\' can be used with any expression\n      }*2\n\n      0.0: Leaf{}*4~6.5 # This will be rounded to an integer\n   }\n}\n\nBranch_A {\n   curve = sp_BA # Identifiers are defined below\n\n   # The parent operator \'^\' gives you access to the branch\'s parent\n   scale = ^.scale * 0.57\n\n   angle = 120\n   ticks = parent.ticks # \'parent\' is the same as \'^\'\n}\n\nBranch_B {\n   curve= sp_BB\n   scale= ^.scale * 0.2~0.45\n   angle= 120\n}\n\nLeaf {\n   curve= sp_L\n   scale= 0.3~0.45\n\n   # Emitters can represent any expression\n   # You can range through identifiers too\n   angle= sixty~seventy\n\n   crection=crec_a\n   # Functions aren\'t re-implemented yet, so scalers need to wait\n   #crectionScaler=scaleFunc()\n}\n\n# Here is where our emitters are defined\n# Nesting emitters doesn\'t work yet, eg \"emit=[one, two]\"\nsp_BA = [0.0, 0.0, 0.0, 0.0, 1.0, 0.001, 0.5, 2.0, 0.8, 2.0, 3.0, 1.2]\nsp_BB = [0.0, 0.0, 0.0, 2, 3, -0.5~0.5, 4, 5, -0.5~0.5, 6, 2, -0.5~0.5]\nsp_L=[0.0, 0.0, 0.0, 2~5.5, 3, 3, 4, 5, 0~3, 6, 2, 0~3]\nsixty = 60\nseventy = 70\ncrec_a = [1, 0, 0, 0, 0.2, 0, -1, 0, 0, 0, -0.2, 0]';
    } else {
        const text = window.localStorage.getItem(name);
        if (text === null) {
            log('No program "' + name + '" found in local storage');
        }

        codeField.value = text;
    }

    log('Loaded "' + name + '"');
}

function saveCode(name) {
    if(isReserved(name)) {
        log('WARNING: Save failed. Name "' + name + '" is reserved.');
        return;
    }

    try {
        window.localStorage.setItem(name, codeField.value);
        log('Saved as "' + name + '".');
    } catch(e) {
        log('WARNING: Unable to save program "' + name + '"!');
    }
}

// Save work!
function handleUnload() {
    window.localStorage.setItem('lastsession', codeField.value);
}

function loadInitialProgram() {
    const code = window.localStorage.getItem('lastsession');
    if(!code) {
        loadCode('default');
    } else {
        codeField.value = code;
    }
}

Module.onRuntimeInitialized = () => {
    // Converted using http://www.howtocreate.co.uk/tutorials/jsexamples/syntax/prepareInline.html
    logField.value = 'Welcome to CosmoGarden!';

    loadInitialProgram();


    // Allow tab in textarea
    codeField.addEventListener('keydown', function (e) {
        if (e.code === 'Tab') {
            e.preventDefault();
            let s = this.selectionStart;
            this.value = this.value.substring(0, this.selectionStart) + TAB + this.value.substring(this.selectionEnd);
            this.selectionEnd = s + TAB.length;
        }
    });

    canvas.addEventListener('click', () => {
        Module.enableKeyboard();
    });

    canvas.addEventListener('mouseleave', () => {
        Module.disableKeyboard();
    });

    cmdField.addEventListener('keydown', e => {
        if(e.code === 'Enter') {
            executeCommand(cmdField.value);
            cmdField.value = '';
        }
    });

    parseBtn.addEventListener('click', handleParse);
    runBtn.addEventListener('click', handleRun);

    window.addEventListener('beforeunload', handleUnload);

    Module.initAll();
};

function executeCommand(text) {
    const [cmd, ...args] = text.trim().split(' ');
    if(cmd.length === 0) return;

    switch(cmd) {
        case 'save':
            if(args.length !== 1) {
                log('Usage: save <name>');
                return;
            }

            saveCode(args[0]);
            break;
        case 'load':
            if(args.length !== 1) {
                log('Usage: load <name>');
                return;
            }

            loadCode(args[0]);
            break;
        default:
            log('Invalid command: ' + cmd);
    }
}

function makeVector(initializer, arr) {
    const vec = initializer();
    arr.forEach(e => {
       vec.push_back(e);
    });

    return vec;
}

function vertexify(arr) {
    if(arr.length % 3 !== 0) throw new Error('Array length not divisible by 3');
    const ret = [];
    for(let i = 0; i < arr.length; i += 3) {
        ret.push({
            x: arr[i],
            y: arr[i + 1],
            z: arr[i + 2]
        });
    }
    return ret;
}

function registerBranch(branch) {
    let bd = {};
    bd.id = branchScalers.registerScaler(branch.crectionScale);
    bd.crossSection = makeVector(Module.makeVertexVector, vertexify(branch.crection));
    bd.curve = branch.curve;
    bd.startRatio = branch.start;
    bd.scale = branch.scale;
    bd.angle = branch.angle;
    bd.ticksToGrow = branch.ticks;
    bd.children = makeVector(Module.makeBranchVector, branch.children.map(child => registerBranch(child)));

    return bd;
}
