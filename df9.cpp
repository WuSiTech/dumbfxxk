// df9.cpp
// DF9 v6 -> x86-64 Linux / Windows AT&T assembly
// g++ -std=c++17 df9.cpp -o df9

#include <bits/stdc++.h>
using namespace std;

#if defined(_WIN32) || defined(_WIN64)
const bool IS_WIN = true;
#else
const bool IS_WIN = false;
#endif

struct GlobalSym {
	bool isArray = false;
	int size = 1;
};

struct LocalSym {
	int offset = 0;
	bool isArray = false;
	int size = 1;
};

class Compiler {
public:
	string src;
	size_t pos = 0;
	
	map<string, GlobalSym> globals;
	map<string, LocalSym> locals;
	
	vector<string> funcNames;
	vector<string> funcDefs;
	vector<string> extraFuncs;
	string globalAsm;
	vector<string> strings;
	
	int labelCnt = 0;
	
	string curFunc;
	string body;
	int stackOff = 0;
	int maxStack = 0;
	string retLabel;
	vector<pair<string, string>> loopStack;
	bool inFunc = false;
	
	Compiler(string s) : src(std::move(s)) {}
	
	[[noreturn]] void err(const string &msg) {
		cerr << "DF9 error: " << msg << " at pos " << pos << "\n";
		if (pos < src.size()) cerr << "near: " << src.substr(pos, 40) << "\n";
		exit(1);
	}
	
	void skip() {
		while (pos < src.size()) {
			// 自动跳过 Windows 记事本默认的 BOM 头
			if (pos == 0 && src.size() >= 3 && (unsigned char)src[0] == 0xEF && (unsigned char)src[1] == 0xBB && (unsigned char)src[2] == 0xBF) {
				pos += 3;
			} else if (isspace((unsigned char)src[pos])) {
				pos++;
			} else if (src[pos] == '/' && pos + 1 < src.size() && src[pos + 1] == '/') {
				while (pos < src.size() && src[pos] != '\n') pos++;
			} else if (src[pos] == '/' && pos + 1 < src.size() && src[pos + 1] == '*') {
				pos += 2;
				while (pos + 1 < src.size() && !(src[pos] == '*' && src[pos + 1] == '/')) pos++;
				if (pos + 1 < src.size()) pos += 2;
			} else {
				break;
			}
		}
	}
	
	char peek() {
		skip();
		return pos < src.size() ? src[pos] : '\0';
	}
	
	bool eat(char c) {
		skip();
		if (pos < src.size() && src[pos] == c) {
			pos++;
			return true;
		}
		return false;
	}
	
	void expect(char c) {
		skip();
		if (pos >= src.size() || src[pos] != c) {
			string g = pos < src.size() ? string(1, src[pos]) : string("$");
			err(string("expect '") + c + "' got '" + g + "'");
		}
		pos++;
	}
	
	string readNumStr() {
		skip();
		string s;
		while (pos < src.size() && isdigit((unsigned char)src[pos])) {
			s += src[pos++];
		}
		return s;
	}
	
	int readNum() {
		string s = readNumStr();
		if (s.empty()) return 0;
		return stoi(s);
	}
	
	string readIdentN(int n) {
		skip();
		string s;
		for (int i = 0; i < n && pos < src.size(); i++) {
			s += src[pos++];
		}
		return s;
	}
	
	string readIdent() {
		skip();
		string s;
		while (pos < src.size() &&
			   (isalnum((unsigned char)src[pos]) || src[pos] == '_')) {
			s += src[pos++];
		}
		return s;
	}
	
	string readUntil(char c) {
		string s;
		while (pos < src.size() && src[pos] != c) {
			s += src[pos++];
		}
		return s;
	}
	
	void emit(const string &s) {
		body += s;
		if (s.empty() || s.back() != '\n') body += '\n';
	}
	
	string addString(const string &s) {
		strings.push_back(s);
		return ".LC" + to_string(strings.size() - 1);
	}
	
	static string esc(const string &s) {
		string r;
		for (char c : s) {
			if (c == '\\') r += "\\\\";
			else if (c == '"') r += "\\\"";
			else if (c == '\n') r += "\\n";
			else if (c == '\t') r += "\\t";
			else r += c;
		}
		return r;
	}
	
	void allocLocal(const string &name, bool arr, int size) {
		if (locals.count(name)) {
			err("local redeclared: " + name);
		}
		int bytes = size * 4;
		stackOff -= bytes;
		LocalSym ls;
		ls.offset = stackOff;
		ls.isArray = arr;
		ls.size = size;
		locals[name] = ls;
		maxStack = max(maxStack, -stackOff);
	}
	
	// ---------- expression ----------
	
	void genExpr() {
		genRel();
	}
	
	void genRel() {
		genAdd();
		while (true) {
			skip();
			char c = peek();
			if (c == ']' || c == '[' || c == '=' || c == '!' || c == '{' || c == '}') {
				char op = c;
				pos++;
				emit("push %rax");
				genAdd();
				emit("pop %rcx");
				emit("cmp %eax,%ecx");
				string st;
				if (op == ']') st = "setl";
				else if (op == '[') st = "setg";
				else if (op == '=') st = "sete";
				else if (op == '!') st = "setne";
				else if (op == '{') st = "setle";
				else st = "setge";
				emit(st + " %al");
				emit("movzbl %al,%eax");
			} else {
				break;
			}
		}
	}
	
	void genAdd() {
		genMul();
		while (true) {
			skip();
			char c = peek();
			if (c == '+' || c == '-') {
				char op = c;
				pos++;
				emit("push %rax");
				genMul();
				emit("pop %rcx");
				if (op == '+') {
					emit("add %ecx,%eax");
				} else {
					emit("sub %eax,%ecx");
					emit("mov %ecx,%eax");
				}
			} else {
				break;
			}
		}
	}
	
	void genMul() {
		genPrimary();
		while (true) {
			skip();
			char c = peek();
			if (c == '*' || c == '/' || c == '%') {
				char op = c;
				pos++;
				emit("push %rax");
				genPrimary();
				emit("pop %rcx");
				if (op == '*') {
					emit("imul %ecx,%eax");
				} else {
					emit("mov %eax,%r10d");
					emit("mov %ecx,%eax");
					emit("cltd");
					emit("idiv %r10d");
					if (op == '%') emit("mov %edx,%eax");
				}
			} else {
				break;
			}
		}
	}
	
	void genPrimary() {
		skip();
		if (eat('(')) {
			genExpr();
			expect(')');
			return;
		}
		if (peek() == '-') {
			pos++;
			genPrimary();
			emit("neg %eax");
			return;
		}
		if (isdigit((unsigned char)peek())) {
			int v = readNum();
			emit("mov $" + to_string(v) + ",%eax");
			return;
		}
		// 支持在表达式中调用函数
		if (peek() == 'F') {
			parseF();
			return;
		}
		if (isalpha((unsigned char)peek()) || peek() == '_') {
			string name = readIdent();
			skip();
			if (peek() == '#') {
				pos++;
				genExpr();
				
				auto it = locals.find(name);
				bool isLocal = it != locals.end();
				if (!isLocal && !globals.count(name)) {
					err("unknown array: " + name + ". Did you forget global declaration like DI4SRC*65536; ?");
				}
				
				emit("mov %eax,%ecx");
				emit("movslq %ecx,%rcx");
				
				if (isLocal) {
					emit("lea " + to_string(it->second.offset) + "(%rbp),%rbx");
				} else {
					emit("lea " + name + "(%rip),%rbx");
				}
				emit("movl (%rbx,%rcx,4),%eax");
				return;
			}
			
			auto it = locals.find(name);
			if (it != locals.end()) {
				emit("mov " + to_string(it->second.offset) + "(%rbp),%eax");
			} else if (globals.count(name)) {
				emit("mov " + name + "(%rip),%eax");
			} else {
				err("unknown variable: " + name);
			}
			return;
		}
		err("bad expression");
	}
	
	// ---------- assignment / local decl ----------
	
	void genAssign() {
		string name = readIdent();
		if (name.empty()) err("assignment missing lhs");
		
		skip();
		if (peek() == '#') {
			pos++;
			genExpr();
			
			emit("mov %eax,%ecx");
			emit("movslq %ecx,%rcx");
			
			auto it = locals.find(name);
			if (it != locals.end()) {
				emit("lea " + to_string(it->second.offset) + "(%rbp),%r11");
			} else if (globals.count(name)) {
				emit("lea " + name + "(%rip),%r11");
			} else {
				err("unknown array lhs: " + name + ". Did you forget global declaration like DI4SRC*65536; ?");
			}
			emit("lea (%r11,%rcx,4),%r11");
			
			expect('&');
			genExpr();
			emit("mov %eax,(%r11)");
			return;
		}
		
		expect('&');
		genExpr();
		
		auto it = locals.find(name);
		if (it != locals.end()) {
			emit("mov %eax," + to_string(it->second.offset) + "(%rbp)");
		} else if (globals.count(name)) {
			emit("mov %eax," + name + "(%rip)");
		} else {
			err("unknown variable lhs: " + name);
		}
	}
	
	void parseLocalDecl() {
		char type = peek();
		if (!isalpha((unsigned char)type)) err("local decl missing type");
		pos++;
		
		int len = readNum();
		string name = readIdentN(len);
		if (name.empty()) err("local decl missing name");
		
		skip();
		if (peek() == '*') {
			pos++;
			int size = readNum();
			if (size <= 0) err("bad local array size");
			allocLocal(name, true, size);
			return;
		}
		
		allocLocal(name, false, 1);
		
		skip();
		if (peek() != ';' && peek() != '\'' && peek() != ',' && peek() != '\0') {
			genExpr();
			emit("mov %eax," + to_string(locals[name].offset) + "(%rbp)");
		}
	}
	
	// ---------- control flow / calls ----------
	
	void parseBlockUntilQuote() {
		while (true) {
			skip();
			if (peek() == '\'') {
				pos++;
				break;
			}
			if (peek() == '\0') break;
			parseStatement();
			skip();
			if (peek() == ';') pos++;
		}
	}
	
	bool tryParseElse() {
		skip();
		if (peek() != 'F') return false;
		size_t save = pos;
		pos++;
		int l = readNum();
		if (l != 2) {
			pos = save;
			return false;
		}
		string w = readIdentN(2);
		if (w != "EL") {
			pos = save;
			return false;
		}
		skip();
		if (peek() != ',') {
			pos = save;
			return false;
		}
		pos++;
		return true;
	}
	
	void genInlineStmt(const string &s) {
		string oldSrc = src;
		size_t oldPos = pos;
		src = s;
		pos = 0;
		parseStatement();
		src = oldSrc;
		pos = oldPos;
	}
	
	void genInlineExpr(const string &s) {
		string oldSrc = src;
		size_t oldPos = pos;
		src = s;
		pos = 0;
		genExpr();
		src = oldSrc;
		pos = oldPos;
	}
	
	void parseF() {
		expect('F');
		int len = readNum();
		string w = readIdentN(len);
		
		if (w == "IF") {
			int n = labelCnt++;
			string le = ".Lelse_" + to_string(n);
			string lend = ".Lend_" + to_string(n);
			
			genExpr();
			expect(',');
			
			emit("cmp $0,%eax");
			emit("je " + le);
			
			parseBlockUntilQuote();
			
			if (tryParseElse()) {
				emit("jmp " + lend);
				emit(le + ":");
				parseBlockUntilQuote();
				emit(lend + ":");
			} else {
				emit(le + ":");
			}
			return;
		}
		
		if (w == "W") {
			int n = labelCnt++;
			string begin = ".Lbegin_" + to_string(n);
			string end = ".Lend_" + to_string(n);
			
			emit(begin + ":");
			genExpr();
			expect(',');
			emit("cmp $0,%eax");
			emit("je " + end);
			
			loopStack.push_back({begin, end});
			parseBlockUntilQuote();
			loopStack.pop_back();
			
			emit("jmp " + begin);
			emit(end + ":");
			return;
		}
		
		if (w == "FR") {
			int n = labelCnt++;
			string begin = ".Lbegin_" + to_string(n);
			string end = ".Lend_" + to_string(n);
			
			string initStr = readUntil('~');
			expect('~');
			string condStr = readUntil('~');
			expect('~');
			string stepStr = readUntil(',');
			expect(',');
			
			genInlineStmt(initStr);
			
			emit(begin + ":");
			genInlineExpr(condStr);
			emit("cmp $0,%eax");
			emit("je " + end);
			
			loopStack.push_back({begin, end});
			parseBlockUntilQuote();
			loopStack.pop_back();
			
			genInlineStmt(stepStr);
			emit("jmp " + begin);
			emit(end + ":");
			return;
		}
		
		if (w == "DW") {
			err("DW do-while is not implemented in this compiler");
		}
		
		if (w == "B") {
			if (loopStack.empty()) err("break outside loop");
			emit("jmp " + loopStack.back().second);
			return;
		}
		
		if (w == "C") {
			if (loopStack.empty()) err("continue outside loop");
			emit("jmp " + loopStack.back().first);
			return;
		}
		
		if (w == "RET") {
			skip();
			if (peek() == ';' || peek() == '\'' || peek() == ',' || peek() == '\0') {
				emit("xor %eax,%eax");
			} else {
				genExpr();
			}
			emit("jmp " + retLabel);
			return;
		}
		
		if (w == "DL") {
			genExpr();
			emit("imul $16666,%eax,%eax");
			if (IS_WIN) {
				emit("mov %eax,%ecx");
				emit("sub $32, %rsp");
				emit("call usleep");
				emit("add $32, %rsp");
			} else {
				emit("mov %eax,%edi");
				emit("call usleep");
			}
			return;
		}
		
		if (w == "PRT") {
			skip();
			if (peek() == '(') {
				pos++;
				genExpr();
				expect(')');
				if (IS_WIN) {
					emit("mov %eax,%edx");
					emit("lea .Lfmt(%rip),%rcx");
					emit("sub $32, %rsp");
					emit("call printf");
					emit("add $32, %rsp");
				} else {
					emit("mov %eax,%esi");
					emit("lea .Lfmt(%rip),%rdi");
					emit("xor %eax,%eax");
					emit("call printf");
				}
				return;
			}
			
			string s;
			if (peek() == '"') {
				pos++;
				while (pos < src.size() && src[pos] != '"') {
					s += src[pos++];
				}
				if (pos < src.size()) pos++;
			} else {
				while (pos < src.size() &&
					   src[pos] != ';' && src[pos] != '\n' && src[pos] != '\'') {
					s += src[pos++];
				}
			}
			string lab = addString(s);
			if (IS_WIN) {
				emit("lea " + lab + "(%rip),%rcx");
				emit("sub $32, %rsp");
				emit("call puts");
				emit("add $32, %rsp");
			} else {
				emit("lea " + lab + "(%rip),%rdi");
				emit("call puts");
			}
			return;
		}
		
		if (w == "JMP") {
			skip();
			string lab;
			while (pos < src.size() &&
				   !isspace((unsigned char)src[pos]) &&
				   src[pos] != ';' && src[pos] != '\'' && src[pos] != ',') {
				lab += src[pos++];
			}
			emit("jmp .L" + lab);
			return;
		}
		
		// 底层 I/O 原语
		if (w == "GET") {
			eat('('); eat(')');
			if (IS_WIN) { emit("sub $32, %rsp"); emit("call __df_get"); emit("add $32, %rsp"); }
			else { emit("call __df_get"); }
			return;
		}
		if (w == "PEEK") {
			eat('('); eat(')');
			if (IS_WIN) { emit("sub $32, %rsp"); emit("call __df_peek"); emit("add $32, %rsp"); }
			else { emit("call __df_peek"); }
			return;
		}
		if (w == "PUT") {
			expect('('); genExpr(); expect(')');
			if (IS_WIN) { emit("mov %eax,%ecx"); emit("sub $32, %rsp"); emit("call __df_put"); emit("add $32, %rsp"); }
			else { emit("mov %eax,%edi"); emit("call __df_put"); }
			return;
		}
		if (w == "EMIT") {
			expect('('); string s;
			if (peek() == '"') { pos++; while (pos < src.size() && src[pos] != '"') s += src[pos++]; if (pos < src.size()) pos++; }
			else { while (pos < src.size() && src[pos] != ')') s += src[pos++]; }
			expect(')'); string lab = addString(s);
			if (IS_WIN) { emit("lea " + lab + "(%rip),%rcx"); emit("sub $32, %rsp"); emit("call __df_emit"); emit("add $32, %rsp"); }
			else { emit("lea " + lab + "(%rip),%rdi"); emit("call __df_emit"); }
			return;
		}
		if (w == "EXIT") {
			eat('('); eat(')');
			if (IS_WIN) { emit("xor %ecx,%ecx"); emit("sub $32, %rsp"); emit("call exit"); emit("add $32, %rsp"); }
			else { emit("xor %edi,%edi"); emit("call exit"); }
			return;
		}
		
		// 普通函数调用
		skip();
		if (peek() != '(') {
			err("function call requires parentheses: " + w);
		}
		pos++;
		
		int nargs = 0;
		if (peek() != ')') {
			while (true) {
				genExpr();
				emit("push %rax");
				nargs++;
				skip();
				if (peek() == ',') {
					pos++;
					continue;
				}
				break;
			}
		}
		expect(')');
		
		if (IS_WIN) {
			if (nargs > 4) err("Win64 ABI: more than 4 args unsupported");
			string wregs[4] = {"%ecx", "%edx", "%r8d", "%r9d"};
			for (int i = nargs - 1; i >= 0; i--) {
				emit("pop %rax");
				emit("mov %eax," + wregs[i]);
			}
			emit("sub $32, %rsp");
			emit("call " + w);
			emit("add $32, %rsp");
		} else {
			if (nargs > 6) err("SysV ABI: more than 6 args unsupported");
			string regs[6] = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"};
			for (int i = nargs - 1; i >= 0; i--) {
				emit("pop %rax");
				emit("mov %eax," + regs[i]);
			}
			emit("call " + w);
		}
	}
	
	void parseLabel() {
		expect('@');
		int len = readNum();
		string name = readIdentN(len);
		emit(".L" + name + ":");
	}
	
	void parseConcurrent() {
		vector<string> thrs;
		vector<string> handles;
		
		while (peek() == '^') {
			pos++;
			
			string oldBody = body;
			body = "";
			parseStatement();
			string stmtAsm = body;
			body = oldBody;
			
			int id = labelCnt++;
			string fn = "__df_thr_" + to_string(id);
			string hd = "__df_thr_h_" + to_string(id);
			
			string f;
			f += ".text\n";
			f += ".globl " + fn + "\n";
			f += fn + ":\n";
			f += " push %rbp\n";
			f += " mov %rsp,%rbp\n";
			f += stmtAsm;
			f += " leave\n";
			f += " ret\n";
			extraFuncs.push_back(f);
			
			globalAsm += ".bss\n.align 8\n" + hd + ":\n .zero 8\n";
			
			thrs.push_back(fn);
			handles.push_back(hd);
		}
		
		for (size_t i = 0; i < thrs.size(); i++) {
			if (IS_WIN) {
				emit("lea " + handles[i] + "(%rip),%rcx");
				emit("xor %edx,%edx");
				emit("lea " + thrs[i] + "(%rip),%r8");
				emit("xor %r9d,%r9d");
				emit("sub $32, %rsp");
				emit("call pthread_create");
				emit("add $32, %rsp");
			} else {
				emit("lea " + handles[i] + "(%rip),%rdi");
				emit("xor %esi,%esi");
				emit("lea " + thrs[i] + "(%rip),%rdx");
				emit("xor %ecx,%ecx");
				emit("call pthread_create");
			}
		}
		for (size_t i = 0; i < thrs.size(); i++) {
			if (IS_WIN) {
				emit("mov " + handles[i] + "(%rip),%rcx");
				emit("xor %edx,%edx");
				emit("sub $32, %rsp");
				emit("call pthread_join");
				emit("add $32, %rsp");
			} else {
				emit("mov " + handles[i] + "(%rip),%rdi");
				emit("xor %esi,%esi");
				emit("call pthread_join");
			}
		}
	}
	
	void parseStatement() {
		skip();
		if (pos >= src.size()) return;
		char c = peek();
		
		if (c == '\'') return;
		if (c == ';') {
			pos++;
			return;
		}
		if (c == '@') {
			parseLabel();
			return;
		}
		if (c == '^') {
			parseConcurrent();
			return;
		}
		if (c == 'D') {
			pos++;
			skip();
			if (peek() == 'F') {
				err("function definition not allowed here");
			}
			parseLocalDecl();
			return;
		}
		if (c == 'F') {
			parseF();
			return;
		}
		
		if (c == 'R' && src.compare(pos, 3, "RET") == 0) {
			pos += 3;
			skip();
			if (peek() == ';' || peek() == '\'' || peek() == ',' || peek() == '\0') {
				emit("xor %eax,%eax");
			} else {
				genExpr();
			}
			emit("jmp " + retLabel);
			return;
		}
		
		genAssign();
	}
	
	void initFunc(const string &name, const vector<pair<char, string>> &params) {
		curFunc = name;
		body = "";
		stackOff = 0;
		maxStack = 0;
		retLabel = ".Lret_" + name;
		locals.clear();
		loopStack.clear();
		inFunc = true;
		
		if (IS_WIN) {
			string wregs[4] = {"%ecx", "%edx", "%r8d", "%r9d"};
			for (size_t i = 0; i < params.size(); i++) {
				if (i >= 4) err("too many args for Win64");
				const string &pname = params[i].second;
				allocLocal(pname, false, 1);
				int off = locals[pname].offset;
				emit("mov " + wregs[i] + "," + to_string(off) + "(%rbp)");
			}
		} else {
			string regs32[6] = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"};
			for (size_t i = 0; i < params.size(); i++) {
				if (i >= 6) err("too many args for SysV");
				const string &pname = params[i].second;
				allocLocal(pname, false, 1);
				int off = locals[pname].offset;
				emit("mov " + regs32[i] + "," + to_string(off) + "(%rbp)");
			}
		}
	}
	
	void finalizeFunc(const string &name) {
		emit(retLabel + ":");
		emit("leave");
		emit("ret");
		
		int stackSize = (maxStack + 15) / 16 * 16;
		
		string f;
		f += ".text\n";
		f += ".globl " + name + "\n";
		if (!IS_WIN) {
			f += ".type " + name + ",@function\n";
		}
		f += name + ":\n";
		f += " push %rbp\n";
		f += " mov %rsp,%rbp\n";
		if (stackSize > 0) {
			f += " sub $" + to_string(stackSize) + ",%rsp\n";
		}
		f += body;
		if (!IS_WIN) {
			f += ".size " + name + ", .-" + name + "\n";
		}
		
		funcDefs.push_back(f);
		inFunc = false;
	}
	
	void parseFunction() {
		char retType = peek();
		if (!isalpha((unsigned char)retType)) err("function missing return type");
		pos++;
		
		int len = readNum();
		string name = readIdentN(len);
		if (name.empty()) err("function missing name");
		
		skip();
		int np = 0;
		if (isdigit((unsigned char)peek())) {
			np = readNum();
		}
		
		vector<pair<char, string>> params;
		for (int i = 0; i < np; i++) {
			char t = peek();
			if (!isalpha((unsigned char)t)) err("param missing type");
			pos++;
			int plen = readNum();
			string pn = readIdentN(plen);
			if (pn.empty()) err("param missing name");
			params.push_back({t, pn});
		}
		
		initFunc(name, params);
		
		skip();
		if (peek() == '\'') {
			pos++;
			parseStatement();
			skip();
			if (peek() == ';') pos++;
			expect('\'');
		} else {
			expect(',');
			while (true) {
				skip();
				if (peek() == '\'') {
					pos++;
					break;
				}
				if (peek() == '\0') break;
				parseStatement();
				skip();
				if (peek() == ';') pos++;
			}
		}
		
		finalizeFunc(name);
		funcNames.push_back(name);
	}
	
	void parseGlobal() {
		char type = peek();
		if (!isalpha((unsigned char)type)) err("global missing type");
		pos++;
		
		int len = readNum();
		string name = readIdentN(len);
		if (name.empty()) err("global missing name");
		
		skip();
		bool arr = false;
		int size = 1;
		string init = "0";
		
		if (peek() == '*') {
			pos++;
			size = readNum();
			if (size <= 0) err("bad global array size");
			arr = true;
		} else {
			skip();
			if (peek() != ';' && peek() != '\'' && peek() != ',' && peek() != '\0') {
				string num;
				while (pos < src.size() &&
					   (isdigit((unsigned char)src[pos]) || src[pos] == '-')) {
					num += src[pos++];
				}
				if (!num.empty()) init = num;
			}
		}
		
		GlobalSym g;
		g.isArray = arr;
		g.size = size;
		globals[name] = g;
		
		if (arr) {
			globalAsm += ".bss\n";
			globalAsm += ".globl " + name + "\n";
			globalAsm += ".align 4\n";
			globalAsm += name + ":\n";
			globalAsm += " .zero " + to_string(size * 4) + "\n";
		} else if (init == "0") {
			globalAsm += ".bss\n";
			globalAsm += ".globl " + name + "\n";
			globalAsm += ".align 4\n";
			globalAsm += name + ":\n";
			globalAsm += " .zero 4\n";
		} else {
			globalAsm += ".data\n";
			globalAsm += ".globl " + name + "\n";
			globalAsm += ".align 4\n";
			globalAsm += name + ":\n";
			globalAsm += " .long " + init + "\n";
		}
	}
	
	void compile() {
		while (true) {
			skip();
			if (pos >= src.size()) break;
			
			if (peek() == 'D') {
				pos++;
				skip();
				if (peek() == 'F') {
					pos++;
					parseFunction();
				} else {
					parseGlobal();
				}
			} else if (peek() == '@') {
				parseLabel();
			} else if (peek() == ';') {
				pos++;
			} else {
				while (pos < src.size() && src[pos] != '\n') pos++;
			}
		}
		
		string out;
		out += ".text\n";
		
		for (const string &n : funcNames) {
			out += ".globl " + n + "\n";
			if (!IS_WIN) {
				out += ".type " + n + ",@function\n";
			}
		}
		
		out += globalAsm;
		
		out += "\n.text\n";
		for (const string &f : funcDefs) {
			out += f;
			out += "\n";
		}
		for (const string &f : extraFuncs) {
			out += f;
			out += "\n";
		}
		
		// 底层 I/O 运行时
		out += "\n.text\n";
		out += "__df_get:\n push %rbp\n mov %rsp,%rbp\n";
		if (!IS_WIN) out += " call getchar\n";
		else out += " sub $32,%rsp\n call getchar\n add $32,%rsp\n";
		out += " leave\n ret\n";
		
		out += "__df_peek:\n push %rbp\n mov %rsp,%rbp\n";
		if (!IS_WIN) out += " call getchar\n push %rax\n mov %eax,%edi\n call ungetc\n pop %rax\n";
		else out += " sub $32,%rsp\n call getchar\n push %rax\n mov %eax,%ecx\n mov $1,%edx\n call ungetc\n pop %rax\n add $32,%rsp\n";
		out += " leave\n ret\n";
		
		out += "__df_put:\n push %rbp\n mov %rsp,%rbp\n";
		if (!IS_WIN) out += " call putchar\n";
		else out += " sub $32,%rsp\n call putchar\n add $32,%rsp\n";
		out += " leave\n ret\n";
		
		out += "__df_emit:\n push %rbp\n mov %rsp,%rbp\n";
		if (!IS_WIN) out += " call puts\n";
		else out += " sub $32,%rsp\n call puts\n add $32,%rsp\n";
		out += " leave\n ret\n";
		
		out += ".section .rodata\n";
		out += ".Lfmt:\n";
		out += " .string \"%d\\n\"\n";
		
		for (size_t i = 0; i < strings.size(); i++) {
			out += ".LC" + to_string(i) + ":\n";
			out += " .string \"" + esc(strings[i]) + "\"\n";
		}
		
		cout << out;
	}
};

int main(int argc, char **argv) {
	ios::sync_with_stdio(false);
	cin.tie(nullptr);
	
	string input;
	if (argc >= 2) {
		ifstream ifs(argv[1]);
		if (!ifs) {
			cerr << "cannot open " << argv[1] << "\n";
			return 1;
		}
		stringstream ss;
		ss << ifs.rdbuf();
		input = ss.str();
	} else {
		stringstream ss;
		ss << cin.rdbuf();
		input = ss.str();
	}
	
	Compiler c(input);
	c.compile();
	return 0;
}
