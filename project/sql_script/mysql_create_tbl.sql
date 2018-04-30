-- 1.教师表
create table TEACHER_TBL(
t_id varchar(20) primary key, -- 教师号
t_name varchar(20) not null, -- 姓名
t_passwd varchar(20) not null
);

-- 2.班级表
create table CLASS_TBL(
cls_no varchar(20) primary key, -- 班级号
cls_name varchar(20) not null -- 名称
);

-- 3.学生表
create table STUDENT_TBL(
s_id varchar(20) primary key, -- 学号
s_name varchar(20) not null, -- 姓名
cls_no varchar(20) not null, -- 班级号
s_passwd varchar(20) not null, -- 密码
constraint `fk_classid` foreign key(cls_no) references CLASS_TBL(cls_no)
);

-- 4.课程表(教师创建二维码, 由老师提供课程编号, 课程名)
create table COURSE_TBL(
course_no varchar(20) primary key, -- 课程号
course_name varchar(20)  not null, -- 课程名
t_id varchar(20) -- 教师号
);

-- 5.选课表(学生扫码, 获取课程号并发送给后台, 
-- 后台判断, 首次选课添加到选课表并记录到签到表, 
-- 后续扫描, 只将记录添加到签到表)
create table SEL_COURSE_TBL(
s_id varchar(20)  not null, -- 学号
course_no varchar(20) primary key, -- 课程号
constraint `fk_SELStudentId` foreign key(s_id) references STUDENT_TBL(s_id)
);

-- 6.签到表
-- 签到号自动增长, 插入时传入NULL即可
create table SINGIN_TBL(
sign_id  INT UNSIGNED AUTO_INCREMENT primary key , -- 签到号
course_no varchar(20)  not null, -- 课程号
s_id varchar(20)  not null, -- 学号
sign_date varchar(15), -- 日期
sign_time varchar(15), -- 时间
constraint `fk_SIGNStudentId` foreign key(s_id) references STUDENT_TBL(s_id),
constraint `fk_SIGNCourseNo` foreign key(course_no) references COURSE_TBL(course_no)
);