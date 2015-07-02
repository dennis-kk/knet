    
	/**
     * 构造函数
     */
    {{@name}}();

    /**
     * 拷贝构造
     * \param rht {{@name}}引用
     */
    {{@name}}(const {{@name}}& rht);

    /**
     * 赋值
     * \param rht {{@name}}引用
     */
    const {{@name}}& operator=(const {{@name}}& rht);

    /**
     * 打印对象
     * \param ss std::stringstream引用， 对象信息将输出到
     * \param white 缩进空格
     */
    void print(std::stringstream& ss, std::string white = "");
