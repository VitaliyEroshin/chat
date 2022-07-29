# Chat
## Requirements
**Tested on**
- MacOS (12.0.1)
- Ubuntu (20.04.3 LTS)

*Some other unix systems might work, but no promises.*
*Windows is not currently supported*

## Quick start
1) If you are running cmake 3.12+ you can build executables as simply as 
```
 ./build.sh
```

However, if you are running older version of cmake, there is another way to do the same thing:
```
mkdir build
cd build
cmake ..
make
```

2) Once executables have successfully been built, you can find your **server** and **client** executables in the ```./bin/``` folder.
3) Firstly, run the **server**.
```
./bin/server
```
 By default, it binds port ***8888***. If there are no errors, output must look like this:
 
<img width="219" alt="Server setup" src="https://user-images.githubusercontent.com/36928556/159168591-443be1a2-fdce-4182-899c-40c9b4402b80.png">

4) Secondly, run the **client**. 
<img width="325" alt="Connecting to host" src="https://user-images.githubusercontent.com/36928556/159168762-0055d056-c163-4650-bf53-9ff1fb5f3eb5.png">
<img width="213" alt="Auth" src="https://user-images.githubusercontent.com/36928556/159168905-d3bc159c-6adb-45ea-9e43-594fb9f948a5.png">
<img width="234" alt="Chatting" src="https://user-images.githubusercontent.com/36928556/159168933-9ee8445f-c100-4f3c-8449-1051a8eae103.png">

5) To view available commands, type ```/help``` command in chat.
6) ***Have fun!*** 🥳

## Roadmap
**Non GUI Client**:
- Registration & authorization (basic)  (**X**)
- Profile editing (nickname, status)
- Chat creation, user invitation        (**X**)
- Switching between available chats     (**X**)
- Text messages                         (**X**)
- Threads from existing messages
- Message history
- Message management (edit & delete)
- Profile view
- Member rights, roles, etc.
