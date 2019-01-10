# *Underverse* - A markdown-based note taking tool

![Alt text](doc/Underverse.png)

## Current state

Underverse is usable, but there is no stable binary release yet.  
*Use at your own risk!*

## Key features

* Portable
* Markdown editor and note-taking tool in one application
* Image/attachment/linking support
* Text search in notes

Explicitly not supported is:

* Synchronizing notes between different devices!  
  Use your favorite clould drive (e.g. GoogleDrive, OneDrive, DropBox) or version control system (e.g. SVN, GIT) for that!


## Obtaining Underverse

Please use git to clone the most recent development version:

    git clone --recursive https://github.com/marc-sturm/Underverse.git

Then, open the `src/Underverse.pro` file in QtCreator and build the application.  
After a successfull build, the binary can be found in the `bin` folder.

## Build notes

 - Qt 5.9.5
 - Download `qtwebkit-5.212.0_alpha2-qt59-mingw530-x86.zip` from <https://github.com/annulen/webkit/releases> and extract it to `C:\Qt\Qt5.9.5\5.9.5\mingw53_32`.


