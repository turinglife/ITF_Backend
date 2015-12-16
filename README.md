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
Useful mysql commands

- create user 'itf'@'localhost' identified by 'password';  # 'root'@'localhost' identified by '123456'
- create database ITF;
- grant all on ITF.* to 'itf';
- SELECT User FROM mysql.user;    # list current users.
- DROP USER 'itf'@'localhost';    # remove an user.
- show databases;
- mysql -u username -p database_name < db/ITF_mysql.sql # mysql -u root -p ITF < db/ITF_mysql.sql
- use ITF;
- show tables;
- SELECT * from one_tables;
- DELETE from one_table; # it will delete all record rows and retain the structure of the table.



