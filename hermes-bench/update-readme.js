const csvToMd = require('csvtomd-lib');
const {promises: {readFile, writeFile}} = require("fs");
const magic = require('markdown-magic');
const path = require('path');

const markdownPath = path.join(__dirname, '../README.md');

Promise.all([
  readFile('arrays.csv'),
  readFile('partial.csv'),
  readFile('persons.csv'),
]).then(([arrays, partial, persons]) => {
  const arraysTable = csvToMd.fromString(arrays.toString());
  const partialTable = csvToMd.fromString(partial.toString());
  const personsTable = csvToMd.fromString(persons.toString());
  const config = {
    transforms: {
      ARRAYS(content, options) {
        return arraysTable;
      },
      PARTIAL(content, options) {
        return partialTable;
      },
      PERSONS(content, options) {
        return personsTable;
      }
    }
  };
  magic(markdownPath, config)
});
