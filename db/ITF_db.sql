
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
    password TEXT
);


-- INSERT VALUES
INSERT INTO Tasks (task_name, type, width, height, address, port, host, username, password)
VALUES ('task_one', 0, 704, 576, '137.189.35.204', 10182, 'mjpg/video.mjpg', 'root', 'xgwangpj');

INSERT INTO Tasks (task_name, type, width, height, address, port, host, username, password)
VALUES ('task_two', 0, 704, 576, '137.189.35.204', 10183, 'mjpg/video.mjpg', 'root', 'xgwangpj');

INSERT INTO Tasks (task_name, type, width, height, address, port, host, username, password)
VALUES ('task_three', 0, 704, 576, '137.189.35.204', 10184, 'mjpg/video.mjpg', 'root', 'xgwangpj');