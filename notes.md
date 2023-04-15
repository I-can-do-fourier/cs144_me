## vscode ssh连接失败

尝试 kill server(kill vs code server on host).重启vm,vscode

如果不行很可能是vm空间不够了


## 扩大容量方法

links: 

https://linux.afnom.net/install_apple/m1.html
https://github.com/utmapp/UTM/issues/2636
[ChatGpt](https://openai.com/blog/chatgpt)

vda结构:
```
NAME                      MAJ:MIN RM  SIZE RO TYPE MOUNTPOINT
vda                       252:0    0   10G  0 disk 
├─vda1                    252:1    0  512M  0 part /boot/efi
├─vda2                    252:2    0    1G  0 part /boot
└─vda3                    252:3    0  8.5G  0 part 
  └─ubuntu--vg-ubuntu--lv 253:0    0  8.5G  0 lvm  /

```

filesystem空间使用情况

```
Filesystem                         Size  Used Avail Use% Mounted on
udev                               1.9G     0  1.9G   0% /dev
tmpfs                              393M  1.4M  392M   1% /run
/dev/mapper/ubuntu--vg-ubuntu--lv  8.4G  5.6G  2.3G  72% /
tmpfs                              2.0G     0  2.0G   0% /dev/shm
tmpfs                              5.0M     0  5.0M   0% /run/lock
tmpfs                              2.0G     0  2.0G   0% /sys/fs/cgroup
/dev/vda2                          976M  108M  802M  12% /boot
/dev/loop1                          58M   58M     0 100% /snap/core20/1084
/dev/vda1                          511M  3.6M  508M   1% /boot/efi
/dev/loop3                          61M   61M     0 100% /snap/lxd/21544
/dev/loop0                          49M   49M     0 100% /snap/core18/2127
/dev/loop2                          62M   62M     0 100% /snap/lxd/21032
/dev/loop4                          29M   29M     0 100% /snap/snapd/12707
/dev/loop5                          29M   29M     0 100% /snap/snapd/12886
http://localhost:9843              212G   48K  212G   1% /home/cs144/shared
tmpfs                              393M     0  393M   0% /run/user/1000
```


### 使用qemu扩充vda容量
```
qemu-img resize $PATH +$SIZE
qemu-img resize Path/<YOU VM>.utm/Images/<YOUR OWN NAME>.qcow2 +100G 
```
### 扩大vda3的容量
```
sudo parted /dev/vda resizepart 3 100%
```
### 扩大vda3的物理存储
```
sudo pvresize /dev/vda3
```
### 扩大ubuntu--vg-ubuntu--lv的逻辑存储
```
sudo lvextend -l +100%FREE /dev/ubuntu-vg/ubuntu-lv
```
### resize文件系统
```
sudo resize2fs /dev/ubuntu-vg/ubuntu-lv
```
### 验证
```
df -h
lsblk /dev/vda
```



