import * as fs from "fs";
import * as child_process from "child_process";

enum ColumnModifier {
  Nothing = " ",
  Diff = "D",
  Sum = "S",
}

interface ColumnInfo
{
  selected: boolean;
  modifier: ColumnModifier;
}

interface Report {
  [column: string]: ColumnInfo;
}

interface Reports {
  [report: string]: Report;
}

interface Data {
  [time: string] : {[column: string]: string};
}


function setupInput() {
  let stdin = process.stdin;

  // without this, we would only get streams once enter is pressed
  (stdin as any).setRawMode(true);

  // resume stdin in the parent process (node app won't quit all by itself
  // unless an error or process.exit() happens)
  stdin.resume();

  // i don't want binary, do you?
  stdin.setEncoding( 'utf8' );
}


function readFile(file: string): [Reports, Data] {
  let reports: Reports = {};
  let data: Data = {};
  let input: string = fs.readFileSync(file, { encoding: "utf-8" });

  for (const line of input.split("\n")) {
    let match = line.match("^([0-9]+): *([a-zA-Z_]+) *:(.*)");
    if (match === null)
      continue;
    const time = match[1];
    const reportName = match[2];
    if (reports[reportName] === undefined)
      reports[reportName] = {};
    let report = reports[reportName];
    if (data[time] === undefined)
      data[time] = {}; 
    let timePoint = data[time];
    
    for (const col of match[3].split(",")) {
      const data = col.split("=");
      if (data.length !== 2)
        continue;
      const column = data[0].trim();
      if (report[column] === undefined) {
        report[column] =  {
          selected: false,
          modifier: ColumnModifier.Nothing,
        };
      }
      timePoint[seriesName(reportName, column)] = data[1].trim();
    }
  }

  return [reports, data];
}

function seriesName(reportName: string, columnName: string) {
  return reportName + "." + columnName;
}


function render(reports: Reports, where: number) {
  let buffer = "";
  let pos = 0;
  const keys = Object.keys(reports);
  const selectedKey = keys[where];
  for (const key of keys) {
    const el = reports[key];
    if (key === selectedKey)
      pos = buffer.length + 1;
    let res =
      (el.selected ? "X" : " ") +
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


async function interactive(reports: Reports) {
  let index = 0;
  render(reports, index);

  const keys = Object.keys(reports);
  const length = keys.length;

  await new Promise((accept) => {
    // on any data into stdin
    let callback = async (key: string) => {
      switch(key) {
        case "\u0003": // ctrl-c
          process.exit();
        case "\u000D": // enter
          process.stdin.off('data', callback);
          await interactiveReport(reports[keys[index]]);
          process.stdin.on('data', callback);
        case "\u001B\u005b\u0044": // left
          if (index > 0)
            index--;
          break;
        case "\u001B\u005b\u0043": // right
          if (index < length - 1)
            index++;
          break;
        case "\u0008": // backspace
          process.stdin.off('data', callback);
          process.stdout.write("\u001b[0G");
          process.stdout.write("\u001b[100X");        
          accept();
      }
      render(reports, index); 
    }
    
    process.stdin.on('data', callback);
  });
}


function renderReport(report: Report, where: number) {
  let buffer = "";
  let pos = 0;
  const keys = Object.keys(report);
  const selectedKey = keys[where];
  for (const key of keys) {
    const el = report[key];
    if (key === selectedKey)
      pos = buffer.length + 1;
    let res =
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


async function interactiveReport(report: Report) {
  let index = 0;
  renderReport(report, index);

  const keys = Object.keys(report);
  const length = keys.length;

  await new Promise((accept) => {
    // on any data into stdin
    let callback = (key: string) => {
      let curr = report[keys[index]];
      switch(key) {
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
    }
    
    process.stdin.on('data', callback);
  });
}


function plot(reports: Reports, data: Data) {
  const columns: string[] = [];
  for (const reportName of Object.keys(reports)) {
    const report = reports[reportName];
    for (const columnName of Object.keys(report)) {
      const column = report[columnName];
      if (column.selected) {
        columns.push(seriesName(reportName, columnName));
      }
    }
  }

  let buffer = "";
  for (const time of Object.keys(data)) {
    buffer = buffer + time;
    const d = data[time];
    for (const col of columns) {
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


async function processData() {
  setupInput();
  const [reports, data] = readFile("sample.txt");
  await interactive(reports);
  plot(reports, data);
  process.exit();
}


processData();