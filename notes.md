## vscode ssh连接失败

尝试 kill server(kill vs code server on host).重启vm,vscode

如果不行很可能是vm空间不够了


## 扩大容量方法

links: 

https://linux.afnom.net/install_apple/m1.html
https://github.com/utmapp/UTM/issues/2636
[ChatGpt](https://openai.com/blog/chatgpt)


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



