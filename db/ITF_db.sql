
-- CREATE TABLES
CREATE TABLE Tasks (
    task_name TEXT PRIMARY KEY,
    type INTEGER NOT NULL, -- It can only has one of HTTP 0, RTSP 1, LOCAL 2, FILE 3
    width INTEGER,
    height INTEGER,

    address TEXT,
    port INTEGER,
    host TEXT,
    username TEXT,
    password TEXT,

    pers_path TEXT,
    roi_path TEXT
);


-- INSERT VALUES
INSERT INTO Tasks (task_name, type, width, height, address, port, host, username, password, pers_path, roi_path)
VALUES ('task_one', 0, 704, 576, '137.189.35.204', 10182, 'mjpg/video.mjpg', 'root', 'xgwangpj', './data/010182_pers.csv', './data/010182_roi.csv');

INSERT INTO Tasks (task_name, type, width, height, address, port, host, username, password, pers_path, roi_path)
VALUES ('task_two', 0, 704, 576, '137.189.35.204', 10183, 'mjpg/video.mjpg', 'root', 'xgwangpj', './data/010183_pers.csv', './data/010183_roi.csv');

INSERT INTO Tasks (task_name, type, width, height, address, port, host, username, password, pers_path, roi_path)
VALUES ('task_three', 0, 704, 576, '137.189.35.204', 10184, 'mjpg/video.mjpg', 'root', 'xgwangpj', './data/010184_pers.csv', './data/010184_roi.csv');

INSERT INTO Tasks (task_name, type, width, height, address, port, host, username, password, pers_path, roi_path)
VALUES ('task_four', 3, 720, 576, './data/200608.mp4', 0, '', '', '', './data/200608_pers.csv', './data/200608_roi.csv');