# c

Automates SSH connections to servers that require a key file and Bastion hosts via a simple JSON configuration file.

## Build instructions (GNU/Linux)

```console
$ mkdir build
$ cd build
$ cmake ..
$ make
$ cp c ~/.local/bin
$ chmod +x ~/.local/bin/c
```

## Config file:

~/.config/c/config.json:

```json
{
	"organizations": {
		"client-1": {
			"key": "/home/user/.ssh/keys/client-1.pem",
			"bastion": "ec2-user@0.0.0.0",
			"servers": {
                "web": "ec2-user@172.31.22.123",
                "api": "ec2-user@172.32.22.124"
			}
        },
        "client-2": {
            "key": "/home/user/.ssh/keys/client-2.pem",
			"bastion": "ec2-user@1.1.1.1",
			"servers": {
                "web": "ec2-user@172.31.22.123",
                "api": "ec2-user@172.32.22.124"
			}
        }
	}
}
```

## Example usage

```console
$ c
usage: c <orginization> <server>
                client-2                web
                client-2                api
                ------------------------------------------------------------
                client-1                web
                client-1                api
                ------------------------------------------------------------

$ c client-1 web
ec2-user@172.31.22.123:~$
```