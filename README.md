# ZSYNC

Zsync is a tool for zfs backups. It allows full and incremental data transfers between hosts.

Zsync was created for internal use, and in my perspective has become a useful tool, so I decided to share it with those who
need lightweight tool for ZFS backups. I'm using it in my hosting company (https://www.hosti24.pl).

## Compilation

zsync requires poco libraries installed. FreeBSD users can install it with command:
```shell
pkg install poco
```

When POCO libraries are ready execute this command inside zsync source directory:
```shell
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Bravo, zsync is ready to work. 

## Running
Running zsync requires one argument - a path to settings file:
```shell
./zsync --config settings.ini
```
## Integration
You can control zsync by simple rest api.

### Backup related commands

* Start backup (same effect as running from timer):
```shell
curl -X PUT "http://<ip addres:port>/backup/run"
```

* Terminate backup:
```shell
curl -X PUT "http://<ip addres:port>/backup/abort"
```

* Show count of queued tasks:

```shell
curl -X GET "http://<ip addres:port>/backup/queue"
```

* Remove old snapshots:
```shell
curl -X PUT "http://<ip addres:port>/backup/removeOld"
```

### Transfer related commands

* List transfers (waiting, running, finished)
```shell
curl -X GET "http://<ip addres:port>/transfer/list"
```
* Forget finished transfers history:
```shell
curl -X PUT "http://<ip addres:port>/transfer/clear"
```

## Configuration
Read sample settings.ini for details.

## Authors and acknowledgment
Paweł Zieliński 

## License
This software is released under MIT License.

I am not responsible for any damage resulting from using this program.

## Project status
I'm still working on  this project. I want to add SSL support for rest api and socket communication betwenn hosts. I'm also planning
to add a compression mechanism for data transfers.

Any questions, issues etc. are welcome. Contact me pawel at zielin .pl.
