
-- To find the specific error run this: SHOW ENGINE INNODB STATUS\G


DROP DATABASE IF EXISTS `ITF`;
CREATE DATABASE `ITF`;


GRANT USAGE ON *.* TO 'itf'@'localhost' IDENTIFIED BY 'itf';
GRANT ALL PRIVILEGES ON `ITF`.* TO 'itf'@'localhost';
FLUSH PRIVILEGES;

USE `ITF`;

CREATE TABLE `Groups` (
    `group_name`        varchar(128),

    PRIMARY KEY (`group_name`)
);

CREATE TABLE Tasks (
    `task_name`         varchar(128),
    
    `task_type`         enum('DENSITY', 'SEGMENTATION' , 'TRACK') NOT NULL,
    `camera_type`       enum('HTTP', 'RTSP' , 'LOCAL', 'FILE') NOT NULL,

    -- The unsigned range is 0 to 65535. 
    `width`             smallint unsigned NOT NULL,  
    `height`            smallint unsigned NOT NULL,

    `address`           varchar(128) NOT NULL,

    `task_path`         varchar(128) NOT NULL,

    `pers_dir`          varchar(32) NOT NULL,
    `roi_dir`           varchar(32) NOT NULL,
    `lm_dir`            varchar(32) NOT NULL,
    `gt_dir`            varchar(32) NOT NULL,
    `alarm_dir`         varchar(32) NOT NULL,

    `pers_file`         varchar(128) NOT NULL,
    `roi_file`          varchar(128) NOT NULL,
    `lm_file`           varchar(128) NOT NULL,

    `alarm_switch`      enum('ON', 'OFF') DEFAULT 'OFF',
    `report_switch`     enum('ON', 'OFF') DEFAULT 'OFF',
    `lm_switch`         enum('ON', 'OFF') DEFAULT 'OFF',

    `group_name`        varchar(128) NOT NULL,

    PRIMARY KEY(`task_name`),
    FOREIGN KEY(`group_name`) REFERENCES Groups(`group_name`) ON DELETE CASCADE
);

CREATE TABLE TaskDetail (
    `task_name`         varchar(128) NOT NULL UNIQUE,
    `host`              varchar(128) NOT NULL,
    `port`              smallint unsigned NOT NULL,  
    `username`          varchar(128) NOT NULL,
    `password`          varchar(128) NOT NULL,

    FOREIGN KEY(`task_name`) REFERENCES Tasks(`task_name`) ON DELETE CASCADE
);

CREATE TABLE AlarmStrategy (
    `task_name`         varchar(128) NOT NULL UNIQUE,
    `priority_low`      smallint unsigned NOT NULL,  
    `priority_medium`   smallint unsigned NOT NULL,  
    `priority_high`     smallint unsigned NOT NULL,  

    FOREIGN KEY(`task_name`) REFERENCES Tasks(`task_name`) ON DELETE CASCADE
);

CREATE TABLE AlarmRecord (
    `task_name`         varchar(128) NOT NULL,
    `date_time`         datetime NOT NULL,
    `count`             int unsigned NOT NULL,
    `priority`          enum('low', 'medium', 'high') NOT NULL,
    `snapshot`          varchar(128) NOT NULL,

    FOREIGN KEY(`task_name`) REFERENCES Tasks(`task_name`) ON DELETE CASCADE
);


-- Insert Values
INSERT INTO Groups VALUES ('mmlab');
INSERT INTO Groups VALUES ('cuhk');
INSERT INTO Tasks VALUES ('task_one', 'DENSITY', 'HTTP', 704, 576, 'mjpg/video.mjpg', 'data/', 'PMap/', 'ROI/', 'LM/', 'GT/', 'Alarm/', '010182_pers.csv', '010182_roi.csv', 'lm.csv', 'ON', 'ON', 'OFF', 'mmlab');
INSERT INTO Tasks VALUES ('task_two', 'DENSITY', 'HTTP', 704, 576, 'mjpg/video.mjpg', '$(HOME)/ITFcs1.0/tasks/task_two/', 'PMap/', 'ROI/', 'LM/', 'GT/', 'Alarm/', 'pers.csv', 'roi.csv', 'lm.csv', 'OFF', 'OFF', 'ON', 'cuhk');
INSERT INTO Tasks VALUES ('task_three', 'SEGMENTATION', 'HTTP', 704, 576, 'mjpg/video.mjpg', 'data/', 'PMap/', 'ROI/', 'LM/', 'GT/', 'Alarm/', 'pers.csv', 'roi.csv', 'lm.csv', 'OFF', 'OFF', 'ON', 'cuhk');
INSERT INTO Tasks VALUES ('task_four', 'SEGMENTATION', 'FILE', 720, 576, '/path/to/200608.mp4', '$(HOME)/ITFcs1.0/tasks/task_four/', 'PMap/', 'ROI/', 'LM/', 'GT/', 'Alarm/', 'pers.csv', 'roi.csv', 'lm.csv', 'OFF', 'OFF', 'OFF', 'mmlab');
INSERT INTO TaskDetail VALUES ('task_one', '137.189.35.204', 10182, 'root', 'xgwangpj');
INSERT INTO TaskDetail VALUES ('task_two', '137.189.35.204', 10183, 'root', 'xgwangpj');
INSERT INTO TaskDetail VALUES ('task_three', '137.189.35.204', 10184, 'root', 'xgwangpj');
INSERT INTO AlarmStrategy VALUES ('task_one', 40, 80, 150);
INSERT INTO AlarmStrategy VALUES ('task_two', 76, 99, 222);
INSERT INTO AlarmRecord VALUES ('task_one', '2012-02-14 18:30:21', 67, 'low', 'c8h3vwn23r');
INSERT INTO AlarmRecord VALUES ('task_one', '2012-02-14 19:20:32', 234, 'high', '5jesvw02kc');
INSERT INTO AlarmRecord VALUES ('task_one', '2012-02-15 07:10:18', 127, 'medium', 'ri5l2ovner');
INSERT INTO AlarmRecord VALUES ('task_one', '2012-02-15 11:40:02', 66, 'low', 'keovjq2ms3');
INSERT INTO AlarmRecord VALUES ('task_one', now(), 88, 'low', 'oc9v2kd9vj');


-- Show
-- SHOW TABLES;
-- SELECT * FROM Groups;
-- SELECT * FROM Tasks;
-- SELECT * FROM TaskDetail;
-- SELECT * FROM AlarmStrategy;
-- SELECT * from AlarmRecord;
