<div align="center">

<img width="150" alt="изображение" src="https://github.com/user-attachments/assets/16915f2a-dfc2-4f88-aa9f-5da29c96c635" />


#  TrochilidaeOctopus



Clients gather telemetry of targets.<br/>
Server has total control on clients.

QML6 / C++20 / SQLite
</div>

## How to
1. Run `OctopusServer.exe`.
2. Press `Add client`. Copy ID and Token. Press `Save`.
3. Open cli and run app  `./OctopusClient.exe -i <ID> -t <Token>`<br/>
   ID and token are cached, so use simply `./OctopusClient.exe` at next runs.<br/>
   You can use run with args again to override credentials.
5. Voila! Connection will be established.<br/>
   Settings of both Client and Server are located at `%APPDATA%/ExperimentalConnectivitySolutions/`

## Features:
1. Multilayer data aggregation (Raw → 10 min → Hour → Day → Week) with compacting approach.

2. Model-View-Controller based server UI.
3. Cliens configuration both .ini and CLI-args ways.
4. Custom protocol with Challenge/Response handshake.
5. Sequence number based protection. Hearbeats (implemented, but not used yet). Automatic reconnection.
6. Shared libs with isolated protocol, serialisation, dispatching, streamer.
7. On-demand extra data loading.
8. Transactions in SQLite for atomic data managent.
9. Strategy-pattern interface to switch ping engines at clients.
10. Server has full control on clients. 
11. C++20 features like concept, named arguments, using enum etc.
12. Demo mode with data flow simulation.
13. Rich logging system.

## TODO:
1. ICMP-engine to collect real telemetry.
2. systemd service wrapping.
3. Move logs inside GUI.
4. Unit tests.
5. Adjust GUI design
6. Adjust compacting logic.
   
<img width="1002" height="756" alt="изображение" src="https://github.com/user-attachments/assets/a2bf1455-4413-49d5-992a-71a3d5744843" />
<img width="1303" height="495" alt="изображение" src="https://github.com/user-attachments/assets/026232ff-d193-42cb-b86d-987b8681be30" />
<img width="1258" height="552" alt="изображение" src="https://github.com/user-attachments/assets/6947e496-e67e-4219-954d-3f1f33dc6fe9" />


