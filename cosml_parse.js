// Token types
const PUNCTUATION_TYPE = 'punctuation';
const KEYWORD_TYPE = 'keyword';
const OPERATOR_TYPE = 'operator';
const NUMBER_TYPE = 'number';
const ID_TYPE = 'identifier';
const COMMENT_CHAR = '#';
const PARENT_CHAR = '^';

// Semantic types
const ROOT_TYPE = 'root';
const BRANCHTEMPLATE_TYPE = 'branch template';
const PLANT_TYPE = 'plant';
const BRANCH_TYPE = 'branch';
const EMITTER_TYPE = 'emitter';

function inputStream(input) {
    let line = 1, pos = 0, col = 0;

    function next() {
        const ch = input.charAt(pos++);
        if (ch === '\n') {
            line++;
            col = 0;
        } else {
            col++;
        }
        return ch;
    }

    function peek() {
        return input.charAt(pos);
    }

    function eof() {
        return peek() === '';
    }

    function err(msg) {
        throw new Error(msg + ' (' + line + ':' + col + ')');
    }

    return {next, peek, eof, err};
}

function tokenStream(input) {
    const keywords = new Set([
        'plant', 'parent',
        'branch', 'curve', 'scale', 'angle', 'ticks', 'start', 'crection', 'crectionScale',
    ]);
    const operators = new Set([
        '-', '+', '*', '/', '<', '>', '~',
    ]);
    const punctuation = new Set([
        ',', ';', ':', '=', '.',
        '(', ')', '[', ']', '{', '}',
    ]);
    const idChars = /[0-9-_]/; // a-zA-Z0-9-_
    const idStartChars = /[a-zA-Z_]/; // a-zA-Z_

    const is = inputStream(input);
    let current = null;

    function isWhitespace(ch) {
        return ' \t\n'.indexOf(ch) >= 0;
    }

    function isDigit(ch) {
        return /[0-9]/.test(ch);
    }

    function isKeyword(word) {
        return keywords.has(word);
    }

    function isPunctuation(ch) {
        return punctuation.has(ch);
    }

    function isOpChar(ch) {
        return operators.has(ch);
    }

    function isIDStart(ch) {
        return idStartChars.test(ch);
    }

    function isID(ch) {
        return isIDStart(ch) || idChars.test(ch);
    }

    function readWhile(predicate) {
        let str = '';
        while (!is.eof() && predicate(is.peek())) {
            str += is.next();
        }
        return str;
    }

    function readID() {
        const id = readWhile(isID);
        return {
            type: isKeyword(id) ? KEYWORD_TYPE : ID_TYPE,
            value: id
        };
    }

    function readNumber() {
        let hasDot = false;
        const number = readWhile(function (ch) {
            if (ch === '.') {
                if (hasDot) return false;
                hasDot = true;
                return true;
            }
            return isDigit(ch);
        });

        return {
            type: NUMBER_TYPE,
            value: number
        };
    }

    function readOperator() {
        return {
            type: OPERATOR_TYPE,
            value: readWhile(isOpChar)
        };

    }

    function skipComment() {
        readWhile(ch => ch !== '\n');
        is.next();
    }

    function readNext() {
        readWhile(isWhitespace);
        if (is.eof()) return null;

        const ch = is.peek();

        if (ch === COMMENT_CHAR) {
            skipComment();
            return readNext();
        }
        if (ch === PARENT_CHAR) {
            is.next();
            return {
                type: KEYWORD_TYPE,
                value: 'parent'
            };
        }
        if (isIDStart(ch)) {
            return readID();
        }
        if (isDigit(ch)) {
            return readNumber();
        }
        if (isOpChar(ch)) {
            return readOperator();
        }
        if (isPunctuation(ch)) {
            return {
                type: PUNCTUATION_TYPE,
                value: is.next()
            };
        }

        is.err('Can\'t read character: ' + ch);
    }

    function next() {
        const tok = current;
        current = null;
        return tok || readNext();
    }

    function peek() {
        if (current === null) current = readNext();
        return current;
    }

    function eof() {
        return peek() === null;
    }

    return {next, peek, eof, err: is.err};
}

function parseCOSML(input) {
    const ts = tokenStream(input);

    function isKeyword(kw) {
        const tok = ts.peek();
        return !ts.eof() && tok.type === KEYWORD_TYPE && tok.value === kw;
    }

    function isPunctuation(ch) {
        const tok = ts.peek();
        return !ts.eof() && tok.type === PUNCTUATION_TYPE && tok.value === ch;
    }

    function isOperator(op) {
        const tok = ts.peek();
        return !ts.eof() && tok.type === OPERATOR_TYPE && tok.value === op;
    }

    function isType(type) {
        return !ts.eof() && ts.peek().type === type;
    }

    function skipPunctuation(ch) {
        if (isPunctuation(ch)) ts.next();
        else wasExpecting('punctuation: "' + ch + '"');
    }

    function skipOperator(op) {
        if (isOperator(op)) ts.next();
        else wasExpecting('operator: "' + op + '"');
    }

    function wasExpecting(expected) {
        ts.err('Expecting ' + expected + ', got ' + JSON.stringify(ts.peek()));
    }

    function expect(type) {
        if (!isType(type)) wasExpecting(type);
    }

    function delimited(start, stop, separator, parser) {
        const values = [];
        let first = true;
        skipPunctuation(start);

        while (!ts.eof()) {
            if (isPunctuation(stop)) break;
            if (first) first = false; else skipPunctuation(separator);
            if (isPunctuation(stop)) break;
            values.push(parser());
        }

        skipPunctuation(stop);
        return values;
    }

    function parseNumber() {
        let numstr = '';
        if (isOperator('-')) {
            skipOperator('-');
            numstr += '-';
        } if(isPunctuation('.')) {
            skipPunctuation('.');
            numstr += '0.';
        }

        expect(NUMBER_TYPE);

        return numstr + ts.next().value;
    }

    function eatBracketed() {
        skipPunctuation('{');
        let depth = 1;
        let next;
        while (true) {
            next = ts.next();
            if (next.type === PUNCTUATION_TYPE) {
                if (next.value === '{') {
                    depth++;
                } else if (next.value === '}') {
                    depth--;
                }

                if (depth === 0) break;
            }
        }
    }

    function parseBranch() {
        const overrides = [];
        const children = [];

        if (!isType(ID_TYPE)) {
            overrides.push(parseEmitter('start'));
            skipPunctuation(':');
        }

        expect(ID_TYPE);
        const template = ts.next().value;
        skipPunctuation('{');

        while (!isPunctuation('}')) {
            if (isType(KEYWORD_TYPE)) overrides.push(parseKeyValue());
            else children.push(parseBranch());
        }

        skipPunctuation('}');

        let repeat;
        if (isOperator('*')) {
            ts.next();
            repeat = parseEmitter();
        } else {
            repeat = {type: EMITTER_TYPE, value: 'return 1;'};
        }

        return {
            type: BRANCH_TYPE,
            template: template,
            children: children,
            overrides: overrides,
            repeat: repeat
        };
    }

    function parsePlant() {
        skipPunctuation('{');

        const branches = [];
        while (!isPunctuation('}')) {
            branches.push(parseBranch());
        }
        skipPunctuation('}');

        return {
            type: PLANT_TYPE,
            value: branches
        };
    }

    function parseKeyValue() {
        expect(KEYWORD_TYPE);
        const key = ts.next().value;
        skipPunctuation('=');

        return parseEmitter(key);
    }

    function parseBranchTemplate(id) {
        skipPunctuation('{');

        const keyValuePairs = [];
        while (!isPunctuation('}')) {
            keyValuePairs.push(parseKeyValue());
        }

        skipPunctuation('}');

        return {
            type: BRANCHTEMPLATE_TYPE,
            id: id,
            value: keyValuePairs
        };
    }

    function parseExpression() {
        let str = '';

        while (true) {
            str += parseAtom();
            if (isType(OPERATOR_TYPE)) {
                str += ts.next().value;
            } else {
                break;
            }
        }

        return str;
    }

    // Parent expressions are accessors
    function parseParentAtom() {
        ts.next(); // skip parent token
        let str = 'parent';
        if (!isPunctuation('.')) {
            wasExpecting('\'.\' following parent keyword');
        }

        while (isPunctuation('.')) {
            skipPunctuation('.');
            expect(KEYWORD_TYPE);
            str += '.' + ts.next().value;
        }

        return str;
    }

    function parseAtom() {
        let str = '';

        if (isPunctuation('[')) {
            const values = delimited('[', ']', ',', parseExpression);
            str += '[';
            values.forEach(s => {
                str += s + ', ';
            });
            str += ']';
        } else if (isPunctuation('(')) {
            skipPunctuation('(');
            str += '(' + parseExpression() + ')';
            skipPunctuation(')');
        } else if (isType(NUMBER_TYPE) || isOperator('-') || isPunctuation('.')) {
            str += parseNumber();
        } else if (isType(ID_TYPE)) {
            const id = ts.next().value;
            str += 'this.data["' + id + '"]()';
        } else if (isKeyword('parent')) {
            str += parseParentAtom();
        } else {
            wasExpecting('value or expression');
        }

        if (isOperator('~')) {
            ts.next();
            str = 'this.randBetween(' + str + ', ' + parseExpression() + ')';
        }

        return str;
    }

    function parseEmitter(id) {
        let str = 'return ' + parseExpression() + ';';

        return {
            type: EMITTER_TYPE,
            value: str,
            id: id
        };
    }

    function parseTop() {
        if (isKeyword('plant')) {
            ts.next();
            return parsePlant();
        }

        if (isType(ID_TYPE)) {
            const id = ts.next().value;

            if (isPunctuation('{')) {
                return parseBranchTemplate(id);
            }
            if (isPunctuation('=')) {
                skipPunctuation('=');
                return parseEmitter(id);
            }

            wasExpecting('"{" or "=" after identifier');
        }

        wasExpecting('plant, branch, or data');
    }

    function parseAll() {
        let expressions = [];
        while (!ts.eof()) {
            const next = parseTop();
            expressions.push(next);
        }
        return {type: ROOT_TYPE, value: expressions};
    }

    return parseAll();
}


// Yields an object that can build a plant
function compileCOSML(input) {
    const plantGenerator = {
        data: {},
        branchTemplates: {},
        randBetween: function (min, max) {
            return Math.random() * (max - min) + min;
        }
    };

    function err(str) {
        throw new Error('Compilation error: ' + str);
    }

    // A reasonable default branch
    function defaultBranch() {
        return {
            start: () => 0.0,
            curve: () => [0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2.0, 0.0, 0.0, 3.0, 0.0],
            scale: () => 1.0,
            angle: () => 120,
            ticks: () => 50,
            crection: () => [1, 0, 0,
                0.707107, 0.707107, 0,
                0, 1, 0,
                -0.707107, 0.707107, 0,
                -1, 0, 0,
                -0.707107, -0.707107, 0,
                0, -1, 0,
                0.707107, -0.707107, 0],
            crectionScale: () =>
                (growthRatio, distAlongCurve) => 0.25 * growthRatio * Math.pow(1.0 - distAlongCurve, 0.4),
        };
    }

    function boundFn(body) {
        return (new Function('parent', body)).bind(plantGenerator);
    }

    // Returns branch with designated attributes overwritten
    function mergeBranch(branch, kv_pairs) {
        kv_pairs.forEach(pair => {
            const key = pair.id;
            if (branch[key]) {
                branch[key] = boundFn(pair.value);
            } else {
                err('invalid branch attribute: ' + JSON.stringify(key));
            }
        });

        return branch;
    }

    function expectArray(result, length = undefined) {
        if (!Array.isArray(result)) {
            throw 'expected array, got ' + JSON.stringify(result);
        }

        if (length && result.length !== length) {
            throw 'expected array of length ' + length + ', was ' + result.length;
        }

        return result;
    }

    function expectInteger(result, forcePositive = false) {
        const errStr = forcePositive ? 'positive integer' : 'integer';
        if (!Number.isInteger(result) || (forcePositive && result < 0)) {
            throw new Error('expected ' + errStr + ', got ' + JSON.stringify(result));
        }

        return result;
    }

    function expectRatio(result) {
        if (typeof result !== 'number' || result < 0.0 || result > 1.0) {
            throw 'expected number between 0 and 1, got ' + JSON.stringify(result);
        }

        return result;
    }

    function expectPositive(result) {
        if (typeof result !== 'number' || result < 0.0) {
            throw 'expected positive number, got ' + JSON.stringify(result);
        }

        return result;
    }

    function positiveInt(result) {
        return Math.round(expectPositive(result));
    }

    function expectFunction(result) {
        if (typeof result !== 'function') {
            throw new Error('expected function, got ' + JSON.stringify(result));
        }

        return result;
    }

    let plant = undefined;
    parseCOSML(input).value.forEach(node => {
        if (node.type === PLANT_TYPE) {
            if (plant) {
                err('multiple definition of plant');
            }
            plant = node;
            return;
        }

        const nodeName = node.id;
        if (node.type === BRANCHTEMPLATE_TYPE) {
            if (plantGenerator.branchTemplates[nodeName]) {
                err('duplicate branch identifier "' + nodeName + '"');
            }
            plantGenerator.branchTemplates[nodeName] = mergeBranch(defaultBranch(), node.value);
        } else if (node.type === EMITTER_TYPE) {
            if (plantGenerator.data[nodeName]) {
                err('duplicate data identifier "' + nodeName + '"');
            }
            plantGenerator.data[nodeName] = boundFn(node.value);
        }
    });

    function resolveTemplate(template, parent) {
        const branch = {};

        branch.start = expectRatio(template.start(parent));
        branch.curve = expectArray(template.curve(parent), 12);
        branch.scale = expectPositive(template.scale(parent));
        branch.angle = expectPositive(template.angle(parent));
        branch.ticks = positiveInt(template.ticks(parent));
        branch.crection = expectArray(template.crection(parent));
        branch.crectionScale = expectFunction(template.crectionScale(parent));
        branch.children = [];

        return branch;
    }

    function resolveBranch(parent, branch) {
        const templateName = branch.template;
        let template = plantGenerator.branchTemplates[templateName];
        if (!template) {
            err('undefined branch template: ' + templateName);
        }

        template = mergeBranch(template, branch.overrides);

        try {
            const repeat = positiveInt(boundFn(branch.repeat.value)(parent));
            let angle = 0;
            for (let i = 0; i < repeat && i < 40; i++) {
                const instance = resolveTemplate(template, parent);
                angle += instance.angle;
                instance.angle = angle;
                branch.children.forEach(child => {
                    resolveBranch(instance, child);
                });
                parent.children.push(instance);

            }
        } catch (e) {
            err('resolving template ' + templateName + ' failed: ' + e);
        }
    }

    // This constructs and returns the final plant by recursively resolving all emitters
    plantGenerator.build = function () {
        if (!plant) err('no plant defined');

        const root = resolveTemplate(defaultBranch());
        plant.value.forEach(branch => {
            resolveBranch(root, branch);
        });

        return root.children;
    };

    return plantGenerator;
}