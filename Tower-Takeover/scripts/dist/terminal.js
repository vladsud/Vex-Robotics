"use strict";
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : new P(function (resolve) { resolve(result.value); }).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
var __generator = (this && this.__generator) || function (thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g;
    return g = { next: verb(0), "throw": verb(1), "return": verb(2) }, typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (_) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
};
exports.__esModule = true;
var fs = require("fs");
var ColumnModifier;
(function (ColumnModifier) {
    ColumnModifier["Nothing"] = " ";
    ColumnModifier["Diff"] = "D";
    ColumnModifier["Sum"] = "S";
})(ColumnModifier || (ColumnModifier = {}));
function setupInput() {
    var stdin = process.stdin;
    // without this, we would only get streams once enter is pressed
    stdin.setRawMode(true);
    // resume stdin in the parent process (node app won't quit all by itself
    // unless an error or process.exit() happens)
    stdin.resume();
    // i don't want binary, do you?
    stdin.setEncoding('utf8');
}
function readFile(file) {
    var reports = {};
    var data = {};
    var input = fs.readFileSync(file, { encoding: "utf-8" });
    for (var _i = 0, _a = input.split("\n"); _i < _a.length; _i++) {
        var line = _a[_i];
        var match = line.match("^([0-9]+): *([a-zA-Z_]+) *:(.*)");
        if (match === null)
            continue;
        var time = match[1];
        var reportName = match[2];
        if (reports[reportName] === undefined)
            reports[reportName] = {};
        var report = reports[reportName];
        if (data[time] === undefined)
            data[time] = {};
        var timePoint = data[time];
        for (var _b = 0, _c = match[3].split(","); _b < _c.length; _b++) {
            var col = _c[_b];
            var data_1 = col.split("=");
            if (data_1.length !== 2)
                continue;
            var column = data_1[0].trim();
            if (report[column] === undefined) {
                report[column] = {
                    selected: false,
                    modifier: ColumnModifier.Nothing
                };
            }
            timePoint[seriesName(reportName, column)] = data_1[1].trim();
        }
    }
    return [reports, data];
}
function seriesName(reportName, columnName) {
    return reportName + "." + columnName;
}
function render(reports, where) {
    var buffer = "";
    var pos = 0;
    var keys = Object.keys(reports);
    var selectedKey = keys[where];
    for (var _i = 0, keys_1 = keys; _i < keys_1.length; _i++) {
        var key = keys_1[_i];
        var el = reports[key];
        if (key === selectedKey)
            pos = buffer.length + 1;
        var res = (el.selected ? "X" : " ") +
            " " +
            key.padEnd(12) +
            " ";
        buffer = buffer + res;
    }
    process.stdout.write("\u001b[0G");
    process.stdout.write("\u001b[100X");
    process.stdout.write(buffer);
    process.stdout.write("\u001b[" + pos + "G");
}
function interactive(reports) {
    return __awaiter(this, void 0, void 0, function () {
        var index, keys, length;
        var _this = this;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    index = 0;
                    render(reports, index);
                    keys = Object.keys(reports);
                    length = keys.length;
                    return [4 /*yield*/, new Promise(function (accept) {
                            // on any data into stdin
                            var callback = function (key) { return __awaiter(_this, void 0, void 0, function () {
                                var _a;
                                return __generator(this, function (_b) {
                                    switch (_b.label) {
                                        case 0:
                                            _a = key;
                                            switch (_a) {
                                                case "\u0003": return [3 /*break*/, 1];
                                                case "\u000D": return [3 /*break*/, 2];
                                                case "\u001B\u005b\u0044": return [3 /*break*/, 4];
                                                case "\u001B\u005b\u0043": return [3 /*break*/, 5];
                                                case "\u0008": return [3 /*break*/, 6];
                                            }
                                            return [3 /*break*/, 7];
                                        case 1:
                                            process.exit();
                                            _b.label = 2;
                                        case 2:
                                            process.stdin.off('data', callback);
                                            return [4 /*yield*/, interactiveReport(reports[keys[index]])];
                                        case 3:
                                            _b.sent();
                                            process.stdin.on('data', callback);
                                            _b.label = 4;
                                        case 4:
                                            if (index > 0)
                                                index--;
                                            return [3 /*break*/, 7];
                                        case 5:
                                            if (index < length - 1)
                                                index++;
                                            return [3 /*break*/, 7];
                                        case 6:
                                            process.stdin.off('data', callback);
                                            process.stdout.write("\u001b[0G");
                                            process.stdout.write("\u001b[100X");
                                            accept();
                                            _b.label = 7;
                                        case 7:
                                            render(reports, index);
                                            return [2 /*return*/];
                                    }
                                });
                            }); };
                            process.stdin.on('data', callback);
                        })];
                case 1:
                    _a.sent();
                    return [2 /*return*/];
            }
        });
    });
}
function renderReport(report, where) {
    var buffer = "";
    var pos = 0;
    var keys = Object.keys(report);
    var selectedKey = keys[where];
    for (var _i = 0, keys_2 = keys; _i < keys_2.length; _i++) {
        var key = keys_2[_i];
        var el = report[key];
        if (key === selectedKey)
            pos = buffer.length + 1;
        var res = 
        // (i === where ? " " : " ") +
        (el.selected ? "X" + el.modifier : "  ") +
            " " +
            key.padEnd(12) +
            " ";
        buffer = buffer + res;
    }
    process.stdout.write("\u001b[0G");
    process.stdout.write("\u001b[100X");
    process.stdout.write(buffer);
    process.stdout.write("\u001b[" + pos + "G");
}
function interactiveReport(report) {
    return __awaiter(this, void 0, void 0, function () {
        var index, keys, length;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    index = 0;
                    renderReport(report, index);
                    keys = Object.keys(report);
                    length = keys.length;
                    return [4 /*yield*/, new Promise(function (accept) {
                            // on any data into stdin
                            var callback = function (key) {
                                var curr = report[keys[index]];
                                switch (key) {
                                    case "\u0003": // ctrl-c
                                        process.exit();
                                    case "\u0008": // backspace
                                        process.stdin.off('data', callback);
                                        accept();
                                        break;
                                    case "\u001B\u005b\u0044": // left
                                        if (index > 0)
                                            index--;
                                        break;
                                    case "\u001B\u005b\u0043": // right
                                        if (index < length - 1)
                                            index++;
                                        break;
                                    case "\u000D": // enter
                                    case " ":
                                        curr.selected = !curr.selected;
                                        break;
                                    case "D":
                                    case "d":
                                        curr.modifier =
                                            curr.selected && curr.modifier == ColumnModifier.Diff ?
                                                ColumnModifier.Nothing :
                                                ColumnModifier.Diff;
                                        curr.selected = true;
                                        break;
                                    case "S":
                                    case "s":
                                        curr.modifier =
                                            curr.selected && curr.modifier == ColumnModifier.Sum ?
                                                ColumnModifier.Nothing :
                                                ColumnModifier.Sum;
                                        curr.selected = true;
                                        break;
                                }
                                renderReport(report, index);
                            };
                            process.stdin.on('data', callback);
                        })];
                case 1:
                    _a.sent();
                    return [2 /*return*/];
            }
        });
    });
}
function plot(reports, data) {
    var columns = [];
    for (var _i = 0, _a = Object.keys(reports); _i < _a.length; _i++) {
        var reportName = _a[_i];
        var report = reports[reportName];
        for (var _b = 0, _c = Object.keys(report); _b < _c.length; _b++) {
            var columnName = _c[_b];
            var column = report[columnName];
            if (column.selected) {
                columns.push(seriesName(reportName, columnName));
            }
        }
    }
    var buffer = "";
    for (var _d = 0, _e = Object.keys(data); _d < _e.length; _d++) {
        var time = _e[_d];
        buffer = buffer + time;
        var d = data[time];
        for (var _f = 0, columns_1 = columns; _f < columns_1.length; _f++) {
            var col = columns_1[_f];
            buffer = buffer + " " + d[col];
        }
        buffer = buffer + "\n";
    }
    console.log(buffer);
    // let p = child_process.exec(`start gnuplot --console`);
    // p.stdin.write("plot '-'");
    // p.stdin.write("1 1");
    //p.stdin.write("2 2");
    //p.stdin.write("e");
}
function processData() {
    return __awaiter(this, void 0, void 0, function () {
        var _a, reports, data;
        return __generator(this, function (_b) {
            switch (_b.label) {
                case 0:
                    setupInput();
                    _a = readFile("sample.txt"), reports = _a[0], data = _a[1];
                    return [4 /*yield*/, interactive(reports)];
                case 1:
                    _b.sent();
                    plot(reports, data);
                    process.exit();
                    return [2 /*return*/];
            }
        });
    });
}
processData();
