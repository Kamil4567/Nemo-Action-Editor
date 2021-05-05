# Nemo-Action-Editor

Simple editor for Nemo Actions. It allows to create, duplicate, rename and delete actions. It can also modify every parameter available in action file(.nemo_action file, example: https://github.com/linuxmint/nemo/blob/master/files/usr/share/nemo/actions/sample.nemo_action) such as: 
- activate or deactivate action, 
- change displayed name, 
- change comment which appear in status bar, 
- command which is executed when user clicks the action, 
- icon name(from system theme) 
- GTK Stock ID(preffered over icon),
- when action shows depending on selection
- when action shows depending on extension of selected files or directories
- when action shows depending on file mime-type
- set dependencies and reverse dependencies
- set custom action show conditions

Editor have 3 themes:
- Light
![Screenshot from 2021-05-05 15-40-23](https://user-images.githubusercontent.com/40038293/117152036-eeb00480-adb9-11eb-8a4e-f9f80350ad95.png)
- Dark
![Screenshot from 2021-05-05 15-40-29](https://user-images.githubusercontent.com/40038293/117152064-f40d4f00-adb9-11eb-8933-2c2fb207369a.png)
- Hacker
![Screenshot from 2021-05-05 15-40-32](https://user-images.githubusercontent.com/40038293/117152090-fa9bc680-adb9-11eb-8b4c-deafcb3bb917.png)

App is created in Qt Creator in C++, all files are in qt_project directory(including icons and images). Compiled executable is in directory compiled(mark as executable and run). Also a deb package can be created(not yet).
