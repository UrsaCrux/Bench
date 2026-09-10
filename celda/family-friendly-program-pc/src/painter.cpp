#include "painter.hpp"



painter::painter(QWidget *parent) : QWidget(parent)
{
	//painter_n.begin(this);
	pen = QPen(Qt::cyan, 1, Qt::SolidLine);
	brush = QBrush(Qt::green, Qt::SolidPattern);

	//painter_n.setPen(pen);
	//painter_n.setBrush(brush);
}

void painter::start_drawing(){
	thread_draw = std::thread(&painter::drawing_listen, this);
	//thread_draw.detach();
	drawing = true;
	//std::cout << "hiiii" << std::endl;
}

void painter::set_connect(serial_listener *listene){
	listen = listene;
	return;
}

void painter::drawing_listen(){
	while(true){
		std::this_thread::sleep_for(std::chrono::milliseconds(1000/100));
		if (listen->new_read){
			if (listen->command_n == "output"){
				listen->new_read = false;
				int num_ = std::stoi(listen->param_n);
				//std::cout << num_ << std::endl;
				numbers.push_back(num_);
				if (num_ > max_){
					max_ = num_;
				}
				//cuando pase de cierta cantidad, hay que
				//empezar a pasar los valores a otro vector
				//y eliminarlos de este, por espacio.
			}
			
		}
		update();
	}
}

void painter::paintEvent(QPaintEvent *event){
	//QGraphicsView::paintEvent(event);
	QPainter p(this);
	p.setPen(pen);
	p.setBrush(brush);

	p.fillRect(rect(), Qt::black);
	
	if (drawing){
		int margin_m = 50;
		int margin = 30;
		int margin_n = 10;
		float diff_v = (height() - (margin_n + margin));
		float diff_h = (width() - (margin_n + margin_m));
		int mid_l = 6;

		//draw axis
		for (int i = 0; i <= steps_v; i++){
			p.drawLine(
				   margin_m-mid_l,
				   height()-margin-(diff_v/steps_v)*i,
				   margin_m,
				   height()-margin-(diff_v/steps_v)*i
				   );
		}

		for (int i = 0; i <= steps_h; i++){
			p.drawLine(
				   margin_m+(diff_h/steps_h)*i,
				   height()-margin+mid_l,
				   margin_m+(diff_h/steps_h)*i,
				   height()-margin);
		}
		int size_n = numbers.size();

		for (int i = 1; i < size_n; i++){
			p.drawLine(
				   (margin_m) + (diff_h/static_cast<float>(size_n))*(i-1),
				   (height() - margin) - (diff_v*numbers[i-1]/(max_ + max_*0.20f)),
				   (margin_m) + (diff_h/static_cast<float>(size_n))*(i),
				   (height() - margin) - (diff_v*numbers[i]/(max_ + max_*0.20f)));
		}

		p.drawLine(margin_m, height()-margin, margin_m, margin_n);
		p.drawLine(margin_m, height()-margin, width()-margin_n, height()-margin);
	}
}
