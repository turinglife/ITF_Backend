###Multi-Task Test on TITAN (Testing by CUHK mmlab)
Dataset: 200608

Note: The network is initialised for each task separately

Analyser Num | Patch Num | Counting FPS | Memory Used
:---: | :---: | :---: | :---:
1 | 171 | 11 | about 320 MB
6 | 171 | 2 | about 2000 MB


Dependencies:

- mysql++ (http://yuanshuilee.blog.163.com/blog/static/217697275201391482712233/)
- mysql (https://www.linode.com/docs/databases/mysql/how-to-install-mysql-on-ubuntu-14-04)


==================================================================
- How to create db using command line

create user 'itf'@'localhost' identified by 'password';

create database ITF;

grant all on ITF.* to 'itf';

