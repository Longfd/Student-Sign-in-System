-- 1.教师表
create table TEACHER_TBL(
t_id varchar(20) primary key, -- 教师号
t_name varchar(20) not null, -- 姓名
t_passwd varchar(20) not null
)ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- 2.班级表
create table CLASS_TBL(
cls_no INT UNSIGNED AUTO_INCREMENT primary key, -- 班级号
cls_name varchar(20) not null, -- 名称
t_id varchar(20)  not null, -- 教师号
constraint `fk_TeacherId` foreign key(t_id) references TEACHER_TBL(t_id)
)ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- 3.学生表
create table STUDENT_TBL(
s_id varchar(20) primary key, -- 学号
s_name varchar(20) not null, -- 姓名
s_passwd varchar(20) not null, -- 密码
cls_no INT UNSIGNED, -- 班级号
cls_name varchar(20), -- 班级名称
constraint `fk_STUClassid` foreign key(cls_no) references CLASS_TBL(cls_no)
)ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- 4.活动表(教师创建二维码, 由老师提供活动名), 活动号需要返回
-- 主键自动增长, 插入时传入NULL即可
create table ACTIVITY_TBL(
act_no INT UNSIGNED AUTO_INCREMENT primary key, -- 活动号
act_name varchar(20)  not null, -- 活动名
t_id varchar(20) not null, -- 教师号
act_class varchar(100) not null, -- 活动班级号
constraint `fk_ATCTid` foreign key(t_id) references TEACHER_TBL(t_id)
)ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- 5.选课表
-- create table SEL_COURSE_TBL(
-- s_id varchar(20)  not null, -- 学号
-- course_no varchar(20) primary key, -- 课程号
-- constraint `fk_SELStudentId` foreign key(s_id) references STUDENT_TBL(s_id)
-- );

-- 6.签到表
-- 由教师创建活动时, 插入表数据
-- 学生扫码, 根据(学号+活动号) 更新签到状态
create table SIGNIN_TBL(
act_no INT UNSIGNED, -- 活动号
act_name varchar(20), -- 活动名
s_id varchar(20) not null, -- 学号
s_name varchar(20), -- 学生姓名
cls_no INT UNSIGNED not null, -- 班级号
cls_name varchar(20) not null, -- 班级名称
sign_date varchar(15), -- 日期
sign_time varchar(15), -- 时间
sign_status INT, -- 签到状态(0-未签到, 1-已签到)
primary key(act_no, s_id),
constraint `fk_SIGNActivityNo` foreign key(act_no) references ACTIVITY_TBL(act_no),
constraint `fk_SIGNStudentId` foreign key(s_id) references STUDENT_TBL(s_id),
constraint `fk_SIGNClassid` foreign key(cls_no) references CLASS_TBL(cls_no)
)ENGINE=InnoDB DEFAULT CHARSET=utf8;

commit;