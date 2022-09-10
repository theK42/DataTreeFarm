# DataTreeFarm
Tool for creating DataTree binary objects from sources such as google sheets.

This is a Node.js project, it requires Node, NPM, CMake, and a C++ compiler available to CMake.

Use `npm install` to grab dependencies and compile. 

It requires a json configuration file for a google sheets API project containing a secret key.  You can generate your own or contact me directly to share the file (credentials.json).  Expected format is identical to google sheets tutorial.

First run of the project will ask for a google login.

To run, either `node . [spreadsheet_id]` or use buildKRBSG.bat.
