
-- To find the specific error run this: SHOW ENGINE INNODB STATUS\G

-- Use InnoDB Engine, which supports foreign key and transaction
-- mysql> show table status;

-- Enable client program to communicate with the server using utf8 character set
SET NAMES 'utf8';

DROP DATABASE IF EXISTS `ITF`;
-- Set the default charset to utf8 for internationalization, use case-insensitive (ci) collation
CREATE DATABASE IF NOT EXISTS `ITF` /*! DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci */;

-- USAGE(Synonym for “no privileges”) can be specified to create a user that has no privileges,
-- or to specify the REQUIRE or WITH clauses for an account without changing its existing privileges.
GRANT USAGE ON *.* TO 'itf'@'localhost' IDENTIFIED BY 'itf';
-- Grant all privileges at specified access level except GRANT OPTION
-- Database Privileges
GRANT ALL ON ITF.* TO 'itf'@'localhost';

FLUSH PRIVILEGES;

-- The new user created has no privileges.
-- You need to grant the appropriate privilege to the user using GRANT command.
-- CREATE USER 'itf'@'localhost' IDENTIFIED BY 'itf';
-- GRANT ALL ON *.* TO 'itf'@'localhost';

USE `ITF`;

CREATE TABLE Groups (
    `group_name`        varchar(128),
    PRIMARY KEY (`group_name`)
) /*! ENGINE=InnoDB */;

CREATE TABLE Tasks (
    `task_name`         varchar(128),
    
    `task_type`         enum('COUNTING', 'SEGMENTATION' , 'TRACK') NOT NULL,
    `task_status`       enum('ON', 'OFF') DEFAULT 'OFF',

    `camera_type`       enum('HTTP', 'RTSP' , 'LOCAL', 'FILE') NOT NULL,
    `camera_status`       enum('ON', 'OFF') DEFAULT 'OFF',

    `task_path`         varchar(128) NOT NULL,
    `alarm_switch`      enum('ON', 'OFF') DEFAULT 'OFF',
    `report_switch`     enum('ON', 'OFF') DEFAULT 'OFF',

    `group_name`        varchar(128) NOT NULL,

    PRIMARY KEY (`task_name`),
    FOREIGN KEY(`group_name`) REFERENCES Groups(`group_name`) ON DELETE CASCADE ON UPDATE CASCADE
) /*! ENGINE=InnoDB */;

CREATE TABLE Cameras (
    `camera_name`   varchar(128),
    -- The unsigned range is 0 to 65535. 
    `width`             smallint unsigned NOT NULL,  
    `height`            smallint unsigned NOT NULL,

    `address`               varchar(128) NOT NULL,
    `host`                      varchar(128),
    `port`                      smallint unsigned,
    `username`          varchar(128),
    `password`          varchar(128),

    PRIMARY KEY (`camera_name`)
) /*! ENGINE=InnoDB */;

CREATE TABLE Task_Camera (
    `task_name`         varchar(128) NOT NULL,
    `camera_name`   varchar(128) NOT NULL,

    FOREIGN KEY(`task_name`) REFERENCES Tasks(`task_name`) ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY(`camera_name`) REFERENCES Cameras(`camera_name`) ON DELETE CASCADE ON UPDATE CASCADE
) /*! ENGINE=InnoDB */;

CREATE TABLE Files (
    `file_url`               varchar(128) NOT NULL,
    -- The unsigned range is 0 to 65535. 
    `width`             smallint unsigned NOT NULL,  
    `height`            smallint unsigned NOT NULL,
    `fps`                  tinyint unsigned NOT NULL,

    `task_name`         varchar(128) NOT NULL UNIQUE,

    FOREIGN KEY(`task_name`) REFERENCES Tasks(`task_name`) ON DELETE CASCADE ON UPDATE CASCADE
) /*! ENGINE=InnoDB */;

CREATE TABLE DensityDetail (
    `pers_file`         varchar(64) NOT NULL,
    `roi_file`          varchar(64) NOT NULL,
    `lm_file`           varchar(64) NOT NULL,

    `task_name`         varchar(128) NOT NULL UNIQUE,

    FOREIGN KEY(`task_name`) REFERENCES Tasks(`task_name`) ON DELETE CASCADE ON UPDATE CASCADE
) /*! ENGINE=InnoDB */;

CREATE TABLE DensityAlarmStrategy (
    `priority_low`      smallint unsigned NOT NULL,  
    `priority_medium`   smallint unsigned NOT NULL,  
    `priority_high`     smallint unsigned NOT NULL,  
    `task_name`         varchar(128) NOT NULL UNIQUE,

    FOREIGN KEY(`task_name`) REFERENCES Tasks(`task_name`) ON DELETE CASCADE ON UPDATE CASCADE
) /*! ENGINE=InnoDB */;

CREATE TABLE DensityAlarmRecord (
    `date_time`     datetime NOT NULL,
    `count`             int unsigned NOT NULL,
    `priority`          enum('LOW', 'MEDIUM', 'HIGH') NOT NULL,
    `snapshot`          varchar(128) NOT NULL,
    `task_name`         varchar(128) NOT NULL,

    FOREIGN KEY(`task_name`) REFERENCES Tasks(`task_name`) ON DELETE CASCADE ON UPDATE CASCADE
) /*! ENGINE=InnoDB */;


-- Insert Values
INSERT INTO Groups VALUES ('mmlab');
INSERT INTO Groups VALUES ('cuhk');

INSERT INTO Tasks VALUES ('task_one', 'COUNTING', 'OFF', 'HTTP', 'OFF', 'data/', 'ON', 'ON', 'mmlab');
INSERT INTO Tasks VALUES ('task_two', 'COUNTING', 'OFF', 'HTTP', 'OFF', '$(HOME)/ITFcs1.0/tasks/task_two/', 'ON', 'ON', 'cuhk');
INSERT INTO Tasks VALUES ('task_three', 'SEGMENTATION', 'OFF', 'HTTP', 'OFF', 'data/', 'ON', 'ON', 'cuhk');
INSERT INTO Tasks VALUES ('task_four', 'SEGMENTATION', 'OFF', 'FILE', 'OFF', '$(HOME)/ITFcs1.0/tasks/task_four/', 'ON', 'ON', 'mmlab');

INSERT INTO Cameras VALUES ('cam_10182', 704, 576, 'mjpg/video.mjpg', '137.189.35.204', 10182, 'root', 'xgwangpj');
INSERT INTO Cameras VALUES ('cam_10183', 704, 576, 'mjpg/video.mjpg', '137.189.35.204', 10183, 'root', 'xgwangpj');
INSERT INTO Cameras VALUES ('cam_10184', 704, 576, 'mjpg/video.mjpg', '137.189.35.204', 10184, 'root', 'xgwangpj');

INSERT INTO Task_Camera VALUES ('task_one', 'cam_10182');
INSERT INTO Task_Camera VALUES ('task_two', 'cam_10183');
INSERT INTO Task_Camera VALUES ('task_three', 'cam_10184');

INSERT INTO Files VALUES ('./data/200608.mp4', 720, 576, 50, 'task_four');

INSERT INTO DensityDetail VALUES ('task_one_pers.csv', 'task_one_roi.csv', 'task_one_lm.csv', 'task_one');
INSERT INTO DensityDetail VALUES ('task_two_pers.csv', 'task_two_roi.csv', 'task_two_lm.csv', 'task_two');

INSERT INTO DensityAlarmStrategy VALUES (40, 80, 150, 'task_one');
INSERT INTO DensityAlarmStrategy VALUES (76, 99, 222, 'task_two');

INSERT INTO DensityAlarmRecord VALUES ('2012-02-14 18:30:21', 67, 'LOW', 'c8h3vwn23r', 'task_one');
INSERT INTO DensityAlarmRecord VALUES (now(), 88, 'LOW', 'oc9v2kd9vj', 'task_one');


-- -- Show
-- SHOW TABLES;
SELECT * FROM Groups;
SELECT * FROM Tasks;
SELECT * FROM Cameras;
SELECT * FROM Task_Camera;
SELECT * FROM Files;
SELECT * FROM DensityDetail;
SELECT * FROM DensityAlarmStrategy;
SELECT * from DensityAlarmRecord;
