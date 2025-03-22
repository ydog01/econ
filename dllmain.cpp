#include"pch.h"

#define RYLog(Rdata,Ydata) std::cerr << "\033[38;2;255;0;0mError: "<<Rdata<<" \033[38;2;200;155;100m" << Ydata << "\033[38;2;255;0;0m !\n\033[38;2;0m"
#define BGOut(str) std::cout<<"\033[38;2;0;100;200m"<<str<<"\033[38;2;0m"
#define GBOut(str) std::cout<<"\033[38;2;0;200;100m"<<str<<"\033[38;2;0m"

std::mutex mtx;
std::thread thr;

std::string title("ECON-evaluator");

struct _func_
{
	LivelyPT::colora_t color;
	eval::epre<double> L, R;
	char flag;//= < > } {
};
std::map<std::string, _func_> _funcs_;

std::shared_ptr<sstree<eval::var<double>>> vars(new sstree<eval::var<double>>);
eval::eval<double> eval_([](const char& ch)->bool {return '0' <= ch && ch <= '9'; }, [](const char& ch)->bool {return ('0' <= ch && ch <= '9') || ch == '.'; }, [](const std::string& str) {return std::stod(str); });
std::atomic_bool running = false;
bool mode = 0;

void initeval()
{
	eval_.oper1->insert("-", { 1,-1u,[](const double* arr)->double {return -arr[0]; } });
	eval_.oper1->insert("+", { 1,-1u,[](const double* arr)->double {return arr[0]; } });
	eval_.oper2->insert("+", { 2,0,[](const double* arr)->double {return arr[0] + arr[1]; } });
	eval_.oper2->insert("-", { 2,0,[](const double* arr)->double {return arr[0] - arr[1]; } });
	eval_.oper2->insert("*", { 2,1,[](const double* arr)->double {return arr[0] * arr[1]; } });
	eval_.oper2->insert("/", { 2,1,[](const double* arr)->double {return arr[0] / arr[1]; } });
	eval_.oper2->insert("%", { 2,1,[](const double* arr)->double {return std::fmod(arr[0],arr[1]); } });
	eval_.oper2->insert("^", { 2,2,[](const double* arr)->double {return std::pow(arr[0],arr[1]); } });
	eval_.oper2->insert("<", { 2,2,[](const double* arr)->double {return arr[0] < arr[1]; } });
	eval_.oper2->insert(">", { 2,2,[](const double* arr)->double {return arr[0] > arr[1]; } });
	eval_.oper2->insert("<=", { 2,2,[](const double* arr)->double {return arr[0] <= arr[1]; } });
	eval_.oper2->insert(">=", { 2,2,[](const double* arr)->double {return arr[0] >= arr[1]; } });
	eval_.oper2->insert("==", { 2,2,[](const double* arr)->double {return std::abs(arr[0] - arr[1]) <= 1e-16; } });
	eval_.oper2->insert("!=", { 2,2,[](const double* arr)->double {return std::abs(arr[0] - arr[1]) > 1e-16; } });
	eval_.funcs->insert("sin", { 1,-1u,[](const double* arr)->double {return std::sin(arr[0]); } });
	eval_.funcs->insert("cos", { 1,-1u,[](const double* arr)->double {return std::cos(arr[0]); } });
	eval_.funcs->insert("tan", { 1,-1u,[](const double* arr)->double {return std::tan(arr[0]); } });
	eval_.funcs->insert("sh", { 1,-1u,[](const double* arr)->double {return std::sinh(arr[0]); } });
	eval_.funcs->insert("ch", { 1,-1u,[](const double* arr)->double {return std::cosh(arr[0]); } });
	eval_.funcs->insert("th", { 1,-1u,[](const double* arr)->double {return std::tanh(arr[0]); } });
	eval_.funcs->insert("arsh", { 1,-1u,[](const double* arr)->double {return std::asinh(arr[0]); } });
	eval_.funcs->insert("arch", { 1,-1u,[](const double* arr)->double {return std::acosh(arr[0]); } });
	eval_.funcs->insert("arth", { 1,-1u,[](const double* arr)->double {return std::atanh(arr[0]); } });
	eval_.funcs->insert("arcsin", { 1,-1u,[](const double* arr)->double {return std::asin(arr[0]); } });
	eval_.funcs->insert("arccos", { 1,-1u,[](const double* arr)->double {return std::acos(arr[0]); } });
	eval_.funcs->insert("arctan", { 1,-1u,[](const double* arr)->double {return std::atan(arr[0]); } });
	eval_.funcs->insert("sqrt", { 1,-1u,[](const double* arr)->double {return std::sqrt(arr[0]); } });
	eval_.funcs->insert("log", { 2,-1u,[](const double* arr)->double {return std::log(arr[1]) / log(arr[0]); } });
	eval_.funcs->insert("lg", { 1,-1u,[](const double* arr)->double {return std::log10(arr[0]); } });
	eval_.funcs->insert("ln", { 1,-1u,[](const double* arr)->double {return std::log(arr[0]); } });
	eval_.funcs->insert("exp", { 1,-1u,[](const double* arr)->double {return std::exp(arr[0]); } });
	eval_.funcs->insert("abs", { 1,-1u,[](const double* arr)->double {return std::abs(arr[0]); } });
	eval_.funcs->insert("floor", { 1,-1u,[](const double* arr)->double {return std::floor(arr[0]); } });
	eval_.funcs->insert("ceil", { 1,-1u,[](const double* arr)->double {return std::ceil(arr[0]); } });
	eval_.vars->insert("pi", { eval::vartype::CONSTVAR,M_PI });
	eval_.vars->insert("e", { eval::vartype::CONSTVAR,M_E });
	vars->insert("pi", { eval::vartype::CONSTVAR,M_PI });
	vars->insert("e", { eval::vartype::CONSTVAR,M_E });
	RECT rect;
	GetWindowRect(GetDesktopWindow(), &rect);
	vars->insert("winx", { eval::vartype::FREEVAR,(rect.right >> 1) - 300.0 });
	vars->insert("winy", { eval::vartype::FREEVAR,(rect.bottom >> 1) - 300.0 });
	vars->insert("winw", { eval::vartype::FREEVAR,600.0 });
	vars->insert("winh", { eval::vartype::FREEVAR,600.0 });
	vars->insert("delaytime", { eval::vartype::FREEVAR,17.0 });
	vars->insert("top", { eval::vartype::FREEVAR,5.0 });
	vars->insert("left", { eval::vartype::FREEVAR,-5.0 });
	vars->insert("rectsize", { eval::vartype::FREEVAR,20.0 });
	vars->insert("dxy", { eval::vartype::FREEVAR,1.0 / 3.0 });
	vars->insert("x", { eval::vartype::CONSTVAR,0 });
	vars->insert("y", { eval::vartype::CONSTVAR,0 });
}
winapi int del(std::stringstream& ss)
{
	std::string str;
	ss >> str;
	mtx.lock();
	if (str == "variable")
	{
		ss >> str;
		if (str.substr(0, 5) == std::string("eval."))
		{
			mtx.unlock();
			RYLog("Can\'t delete variable with", "eval.");
			return -1;
		}
		if (!eval_.vars->erase(str))
		{
			mtx.unlock();
			RYLog("Not such variable called", str);
			return -1;
		}
		BGOut("variable delete successfully!\n");
	}
	else if (str == "function")
	{
		ss >> str;
		if (!eval_.funcs->erase(str))
		{
			mtx.unlock();
			RYLog("Not such function called", str);
			return -1;
		}
		BGOut("function delete successfully!\n");
	}
	else
	{
		mtx.unlock();
		RYLog("Wrong kind", str);
		return -1;
	}
	mtx.unlock();
	return 0;
}
winapi int variable(std::stringstream& ss)
{
	std::string str, str1("0");
	eval::epre<double> Lepre;
	size_t pos;
	ss >> str;
	if (str.empty())
	{
		RYLog("Empty name", '\0');
		return -1;
	}
	std::getline(ss, str1);
	str1.erase(std::remove_if(str1.begin(), str1.end(), std::isspace), str1.end());
	mtx.lock();
	if ((pos = eval_.cpre(Lepre, str1)) != -1)
	{
		mtx.unlock();
		RYLog("Wrong expression at", pos);
		return -1;
	}
	auto iter = eval_.vars->search(str);
	if (iter && iter->data)
	{
		if (iter->data->vtype == eval::vartype::CONSTVAR)
		{
			mtx.unlock();
			RYLog("Can't change a const variale :", str);
			return-1;
		}
		try
		{
			iter->data->value = eval_.result(Lepre);
		}
		catch (const std::runtime_error& error)
		{
			mtx.unlock();
			RYLog('\0', error.what());
			return -1;
		}
		BGOut("variale change successfully!\n");
	}
	else
	{
		if (str.substr(0, 5) == std::string("eval."))
		{
			mtx.unlock();
			RYLog("Can\'t create variable with", "eval.");
			return -1;
		}
		try
		{
			eval_.vars->insert(str, { eval::vartype::FREEVAR,eval_.result(Lepre) });
		}
		catch (const std::runtime_error& error)
		{
			mtx.unlock();
			RYLog('\0', error.what());
			return -1;
		}
		BGOut("variale create successfully!\n");
	}
	mtx.unlock();
	return 0;
}
winapi int function(std::stringstream& ss)
{
	static size_t size = 0;
	std::string name, str;
	eval::epre<double> Lepre;
	size_t pos, size_;
	ss >> name >> size_;
	mtx.lock();
	while (size_ > size)
	{
		size++;
		eval_.vars->insert("eval." + std::to_string(size), { eval::vartype::CONSTVAR,0 });
		vars->insert("eval." + std::to_string(size), { eval::vartype::CONSTVAR,0 });
	}
	std::getline(ss, str);
	str.erase(std::remove_if(str.begin(), str.end(), std::isspace), str.end());
	if ((pos = eval_.cpre(Lepre, str)) != -1)
	{
		RYLog("Wrong expression at", pos);
		mtx.unlock();
		return -1;
	}
	eval_.funcs->insert(name, { size_,0,[Lepre,size_](const double* arr)->double
		{
			for (size_t pos = 0; pos < size_; pos++)
				eval_.vars->search("eval." + std::to_string(pos + 1))->data->value = arr[pos];
			return eval_.result(Lepre);
		} });
	BGOut("function add successfully!\n");
	mtx.unlock();
	return 0;
}
winapi int calculate(std::stringstream& ss)
{
	std::string str;
	std::getline(ss, str);
	str.erase(std::remove_if(str.begin(), str.end(), std::isspace), str.end());
	eval::epre<double> Lepre;
	size_t pos;
	mtx.lock();
	if ((pos = eval_.cpre(Lepre, str)) != -1)
	{
		mtx.unlock();
		RYLog("Wrong expression at", pos);
		return -1;
	}
	try
	{
		BGOut(">> ");
		GBOut(eval_.result(Lepre));
		std::cout.put('\n');
	}
	catch (const std::runtime_error& error)
	{
		mtx.unlock();
		RYLog('\0', error.what());
		return -1;
	}
	mtx.unlock();
	return 0;
}
winapi int swap_vars(std::stringstream& sstr)
{
	mtx.lock();
	mode = !mode;
	vars.swap(eval_.vars);
	mtx.unlock();
	BGOut(">> ");
	GBOut("Vars swap now!\n");
	GBOut(std::string("mode now : ") + (mode ? "Window\n" : "Calculate\n"));
	return 0;
}
winapi int del_object(std::stringstream& ss)
{
	mtx.lock();
	std::string str;
	ss >> str;
	if (str.empty())
	{
		_funcs_.clear();
		GBOut("clear successfully!\n");
		mtx.unlock();
		return 0;
	}
	auto it = _funcs_.find(str);
	if (it == _funcs_.end())
	{
		RYLog("Not object called", str);
		mtx.unlock();
		return -1;
	}
	_funcs_.erase(it);
	GBOut("delete successfully!\n");
	mtx.unlock();
}
winapi int add_object(std::stringstream& ss)
{
	mtx.lock();
	if (!mode)
		vars.swap(eval_.vars);
	size_t pos;
	std::string str, rstr, name;
	ss >> name;
	if (_funcs_.find(name) != _funcs_.end())
	{
		_funcs_[name].L.clear();
		_funcs_[name].R.clear();
	}
	ss >> _funcs_[name].color.r >> _funcs_[name].color.g >> _funcs_[name].color.b >> _funcs_[name].color.a;
	std::getline(ss, str);
	str.erase(std::remove_if(str.begin(), str.end(), std::isspace), str.end());
	for (size_t pos = 0; pos < str.size(); pos++)
	{
		if (pos == str.size() - 1)
		{
			RYLog("Wrong expression with not right", "= \\ < \\ > \\ <= \\ >=");
			if (!mode)
				vars.swap(eval_.vars);
			mtx.unlock();
			return -1;
		}
		if (str[pos] == '=')
		{
			rstr = str.substr(pos + 1);
			str = str.substr(0, pos);
			_funcs_[name].flag = '=';
			break;
		}
		else if (str[pos] == '<')
		{
			if (str[pos + 1] == '=')
			{
				if (pos == str.size() - 2)
				{
					RYLog("Wrong expression with not right", "= \\ < \\ > \\ <= \\ >=");
					if (!mode)
						vars.swap(eval_.vars);
					mtx.unlock();
					return -1;
				}
				rstr = str.substr(pos + 2);
				str = str.substr(0, pos);
				_funcs_[name].flag = '{';
			}
			else
			{
				rstr = str.substr(pos + 1);
				str = str.substr(0, pos);
				_funcs_[name].flag = '<';
			}
			break;
		}
		else if (str[pos] == '>')
		{
			if (str[pos + 1] == '=')
			{
				if (pos == str.size() - 2)
				{
					RYLog("Wrong expression with not right", "= \\ < \\ > \\ <= \\ >=");
					if (!mode)
						vars.swap(eval_.vars);
					mtx.unlock();
					return -1;
				}
				rstr = str.substr(pos + 2);
				str = str.substr(0, pos);
				_funcs_[name].flag = '}';
			}
			else
			{
				rstr = str.substr(pos + 1);
				str = str.substr(0, pos);
				_funcs_[name].flag = '>';
			}
			break;
		}
	}
	if ((pos = eval_.cpre(_funcs_[name].L, str)) != -1)
	{
		RYLog("Wrong expression in", str);
		if (!mode)
			vars.swap(eval_.vars);
		mtx.unlock();
		return -1;
	}
	if ((pos = eval_.cpre(_funcs_[name].R, rstr)) != -1)
	{
		RYLog("Wrong expression in", rstr);
		if (!mode)
			vars.swap(eval_.vars);
		mtx.unlock();
		return -1;
	}
	if (!mode)
		vars.swap(eval_.vars);
	mtx.unlock();
	BGOut(">> ");
	GBOut("add paint object now!\n");
	return 0;
}
winapi int hide_window(std::stringstream& ss)
{
	if (!running)
	{
		GBOut("Window already hide!\n");
		return -1;
	}
	running = false;
	BGOut(">> ");
	GBOut("Window hide now!\n");
	return 0;
}
void winmain();
winapi int show_window(std::stringstream& ss)
{
	if (running)
	{
		GBOut("Window already show!\n");
		return -1;
	}
	thr = std::thread(winmain);
	thr.detach();
	running = true;
	BGOut(">> ");
	GBOut("Window show now!\n");
	return 0;

}
winapi int precision(std::stringstream& ss)
{
	size_t size = 4;
	ss >> size;
	std::cout.precision(size);
	return 0;
}
winapi int actions(std::stringstream& sstr)
{
	struct command
	{
		DWORD delaytime, lasttime;
		size_t size;
		double* var;
		eval::epre<double> vepre;
	};
	size_t size;
	sstr >> size;
	std::string str;
	std::map<std::string, std::pair<std::string, HMODULE>>::iterator dlliter;
	std::vector<command> coms(size);
	LivelyPT::SetRGB(100, 200, 0);
	mtx.lock();
	for (size_t size_ = 0; size_ < size; size_++)
	{
		std::cout << "number " << size_ + 1 << "\nvarset : ";
		std::cin >> str;
		if (!eval_.vars->search(str))
		{
			RYLog("Wrong variable", str);
			mtx.unlock();
			return -1;
		}
		coms[size_].var = &eval_.vars->search(str)->data->value;
		std::getline(std::cin, str);
		str.erase(std::remove_if(str.begin(), str.end(), std::isspace), str.end());
		eval_.cpre(coms[size_].vepre, str);
		std::cout << "size : ";
		std::cin >> coms[size_].size;
		std::cout << "delaytime : ";
		std::cin >> coms[size_].delaytime;
		coms.back().lasttime = clock();
	}
	mtx.unlock();
	LivelyPT::ReRGB();
	while (!coms.empty())
	{
		for (auto it = coms.begin(); it != coms.end(); it++)
		{
			if (clock() - it->lasttime >= it->delaytime)
			{
				mtx.lock();
				try
				{
					*it->var = eval_.result(it->vepre);
				}
				catch (...)
				{
					it->size = 1;
				}
				if (--(it->size))
					it->lasttime = clock();
				else
					coms.erase(it--);
				mtx.unlock();
			}
		}
	}
	return 0;
}

void SDL_RenderDrawLineD(SDL_Renderer* rer, double x1, double y1, const double& x2, const double& y2)
{
	static SDL_Rect rect = { 0,0,3,3 };
	size_t num_points = std::hypot(std::abs(x1 - x2), std::abs(y1 - y2));
	double dx = (x2 - x1) / num_points;
	double dy = (y2 - y1) / num_points;
	rect.x = std::round(--x1);
	rect.y = std::round(--y1);
	while (num_points--)
	{
		SDL_RenderDrawRect(rer, &rect);
		x1 += dx;
		y1 += dy;
		rect.x = std::round(x1);
		rect.y = std::round(y1);
	}
}
void winmain()
{
	mtx.lock();
	if (!mode)
		vars.swap(eval_.vars);
	eval::var<double>* winw_ = eval_.vars->search("winw")->data;
	eval::var<double>* winh_ = eval_.vars->search("winh")->data;
	eval::var<double>* winx_ = eval_.vars->search("winx")->data;
	eval::var<double>* winy_ = eval_.vars->search("winy")->data;
	eval::var<double>* rectsize_ = eval_.vars->search("rectsize")->data;
	eval::var<double>* LX = eval_.vars->search("left")->data;
	eval::var<double>* TY = eval_.vars->search("top")->data;
	eval::var<double>* dxy = eval_.vars->search("dxy")->data;
	eval::var<double>* delaytime = eval_.vars->search("delaytime")->data;
	eval::var<double>* x_ = eval_.vars->search("x")->data;
	eval::var<double>* y_ = eval_.vars->search("y")->data;
	if (!mode)
		vars.swap(eval_.vars);
	mtx.unlock();
	double rectsize = rectsize_->value, winw = winw_->value, winh = winh_->value, winx = winx_->value, winy = winy_->value;
	std::vector<std::vector<double>> maptex(winh / rectsize + 1, std::vector<double>(winw / rectsize + 1));
	SDL_Window* win = SDL_CreateWindow(title.c_str(), winx, winy, winw, winh, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* rer = SDL_CreateRenderer(win, -1, 0);
	SDL_SetRenderDrawBlendMode(rer, SDL_BLENDMODE_BLEND);
	Uint64 clk = SDL_GetTicks64();
	SDL_Event event;
	while (running)
	{
		mtx.lock();
		if (!mode)
			vars.swap(eval_.vars);
		if (std::abs(rectsize - rectsize_->value) > 1e-16)
		{
			rectsize = rectsize_->value;
			maptex.resize(winh / rectsize + 1);
			for (auto& it : maptex)
				it.resize(winw / rectsize + 1);
		}
		if (std::abs(winx - winx_->value) > 1e-16 || std::abs(winy - winy_->value) > 1e-16)
		{
			winx = winx_->value;
			winy = winy_->value;
			SDL_SetWindowPosition(win, winx, winy);
		}
		if (std::abs(winw - winw_->value) > 1e-16 || std::abs(winh - winh_->value) > 1e-16)
		{
			winw = winw_->value;
			winh = winh_->value;
			SDL_SetWindowSize(win, winw, winh);
			maptex.resize(winh / rectsize + 1);
			for (auto& it : maptex)
				it.resize(winw / rectsize + 1);
		}
		SDL_SetRenderDrawColor(rer, 255, 255, 255, 255);
		SDL_RenderClear(rer);
		unsigned char flag;
		for (const auto& func : _funcs_)
		{
			try
			{
				y_->value = TY->value;
				for (size_t y = 0; y < maptex.size(); y++)
				{
					x_->value = LX->value;
					for (size_t x = 0; x < maptex[0].size(); x++)
					{
						maptex[y][x] = eval_.result(func.second.L) - eval_.result(func.second.R);
						x_->value += dxy->value;
					}
					y_->value -= dxy->value;
				}
				SDL_SetRenderDrawColor(rer, func.second.color.r, func.second.color.g, func.second.color.b, func.second.color.a);
				for (size_t y = 0; y < maptex.size() - 1; y++)
				{
					for (size_t x = 0; x < maptex[0].size() - 1; x++)
					{
						flag = 0;
						if (std::isnan(maptex[y][x]) || maptex[y][x] == INFINITY)
							flag++;
						if (std::isnan(maptex[y][x + 1]) || maptex[y][x + 1] == INFINITY)
							flag++;
						if (std::isnan(maptex[y + 1][x]) || maptex[y + 1][x] == INFINITY)
							flag++;
						if (std::isnan(maptex[y + 1][x + 1]) || maptex[y + 1][x + 1] == INFINITY)
							flag++;
						if (flag > 1)
							continue;
						flag = 0;
						if (maptex[y][x] >= 0)
						{
							if (func.second.flag == '>' || func.second.flag == '}')
								SDL_RenderDrawPoint(rer, x * rectsize, y * rectsize);
							flag |= 1;
						}
						else if (func.second.flag == '<' || func.second.flag == '{')
							SDL_RenderDrawPoint(rer, x * rectsize, y * rectsize);
						if (maptex[y][x + 1] >= 0)
							flag |= 2;
						if (maptex[y + 1][x] >= 0)
							flag |= 4;
						if (maptex[y + 1][x + 1] >= 0)
							flag |= 8;
						if (func.second.flag == '{' || func.second.flag == '}' || func.second.flag == '=')
							switch (flag)
							{
							case 1:
								SDL_RenderDrawLineD(rer, (x + maptex[y][x] / (maptex[y][x] - maptex[y][x + 1])) * rectsize, y * rectsize, x * rectsize, (y + maptex[y][x] / (maptex[y][x] - maptex[y + 1][x])) * rectsize);
								break;
							case 2:
								SDL_RenderDrawLineD(rer, (x + maptex[y][x] / (maptex[y][x] - maptex[y][x + 1])) * rectsize, y * rectsize, (x + 1) * rectsize, (y + maptex[y][x + 1] / (maptex[y][x + 1] - maptex[y + 1][x + 1])) * rectsize);
								break;
							case 4:
								SDL_RenderDrawLineD(rer, (x + maptex[y + 1][x] / (maptex[y + 1][x] - maptex[y + 1][x + 1])) * rectsize, (y + 1) * rectsize, x * rectsize, (y + maptex[y][x] / (maptex[y][x] - maptex[y + 1][x])) * rectsize);
								break;
							case 8:
								SDL_RenderDrawLineD(rer, (x + maptex[y + 1][x] / (maptex[y + 1][x] - maptex[y + 1][x + 1])) * rectsize, (y + 1) * rectsize, (x + 1) * rectsize, (y + maptex[y][x + 1] / (maptex[y][x + 1] - maptex[y + 1][x + 1])) * rectsize);
								break;

							case 3:
								SDL_RenderDrawLineD(rer, x * rectsize, (y + maptex[y][x] / (maptex[y][x] - maptex[y + 1][x])) * rectsize, (x + 1) * rectsize, (y + maptex[y][x + 1] / (maptex[y][x + 1] - maptex[y + 1][x + 1])) * rectsize);
								break;
							case 5:
								SDL_RenderDrawLineD(rer, (x + maptex[y][x] / (maptex[y][x] - maptex[y][x + 1])) * rectsize, y * rectsize, (x + maptex[y + 1][x] / (maptex[y + 1][x] - maptex[y + 1][x + 1])) * rectsize, (y + 1) * rectsize);
								break;
							case 10:
								SDL_RenderDrawLineD(rer, (x + maptex[y][x] / (maptex[y][x] - maptex[y][x + 1])) * rectsize, y * rectsize, (x + maptex[y + 1][x] / (maptex[y + 1][x] - maptex[y + 1][x + 1])) * rectsize, (y + 1) * rectsize);
								break;
							case 12:
								SDL_RenderDrawLineD(rer, x * rectsize, (y + maptex[y][x] / (maptex[y][x] - maptex[y + 1][x])) * rectsize, (x + 1) * rectsize, (y + maptex[y][x + 1] / (maptex[y][x + 1] - maptex[y + 1][x + 1])) * rectsize);
								break;

							case 6:
								SDL_RenderDrawLineD(rer, (x + maptex[y][x] / (maptex[y][x] - maptex[y][x + 1])) * rectsize, y * rectsize, x * rectsize, (y + maptex[y][x] / (maptex[y][x] - maptex[y + 1][x])) * rectsize);
								SDL_RenderDrawLineD(rer, (x + maptex[y + 1][x] / (maptex[y + 1][x] - maptex[y + 1][x + 1])) * rectsize, (y + 1) * rectsize, (x + 1) * rectsize, (y + maptex[y][x + 1] / (maptex[y][x + 1] - maptex[y + 1][x + 1])) * rectsize);
								break;
							case 9:
								SDL_RenderDrawLineD(rer, (x + maptex[y][x] / (maptex[y][x] - maptex[y][x + 1])) * rectsize, y * rectsize, (x + 1) * rectsize, (y + maptex[y][x + 1] / (maptex[y][x + 1] - maptex[y + 1][x + 1])) * rectsize);
								SDL_RenderDrawLineD(rer, x * rectsize, (y + maptex[y][x] / (maptex[y][x] - maptex[y + 1][x])) * rectsize, (x + maptex[y + 1][x] / (maptex[y + 1][x] - maptex[y + 1][x + 1])) * rectsize, (y + 1) * rectsize);
								break;

							case 7:
								SDL_RenderDrawLineD(rer, (x + maptex[y + 1][x] / (maptex[y + 1][x] - maptex[y + 1][x + 1])) * rectsize, (y + 1) * rectsize, (x + 1) * rectsize, (y + maptex[y][x + 1] / (maptex[y][x + 1] - maptex[y + 1][x + 1])) * rectsize);
								break;
							case 11:
								SDL_RenderDrawLineD(rer, (x + maptex[y + 1][x] / (maptex[y + 1][x] - maptex[y + 1][x + 1])) * rectsize, (y + 1) * rectsize, x * rectsize, (y + maptex[y][x] / (maptex[y][x] - maptex[y + 1][x])) * rectsize);
								break;
							case 13:
								SDL_RenderDrawLineD(rer, (x + maptex[y][x] / (maptex[y][x] - maptex[y][x + 1])) * rectsize, y * rectsize, (x + 1) * rectsize, (y + maptex[y][x + 1] / (maptex[y][x + 1] - maptex[y + 1][x + 1])) * rectsize);
								break;
							case 14:
								SDL_RenderDrawLineD(rer, (x + maptex[y][x] / (maptex[y][x] - maptex[y][x + 1])) * rectsize, y * rectsize, x * rectsize, (y + maptex[y][x] / (maptex[y][x] - maptex[y + 1][x])) * rectsize);
								break;
							}
					}
				}
			}
			catch (...) {}
		}
		SDL_RenderPresent(rer);

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				running = false;
				break;
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_MOVED:
					winx_->value = event.window.data1;
					winy_->value = event.window.data2;
					break;
				case SDL_WINDOWEVENT_RESIZED:
					winw_->value = event.window.data1;
					winh_->value = event.window.data2;
					break;
				}
				break;
			}
		}
		if (!mode)
			vars.swap(eval_.vars);

		mtx.unlock();
		if (SDL_GetTicks64() - clk < delaytime->value)
			SDL_Delay(delaytime->value - SDL_GetTicks64() + clk);
		clk = SDL_GetTicks64();
	}
	SDL_DestroyWindow(win);
	SDL_DestroyRenderer(rer);
}
winapi int _free_(std::stringstream& ss)
{
	running = false;
	return 0;
}
winapi inline std::streambuf* cinbuf(std::streambuf* buf)
{
	return std::cin.rdbuf(buf);
}
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		initeval();
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}
