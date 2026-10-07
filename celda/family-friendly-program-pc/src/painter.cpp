#include "painter.hpp"



painter::painter(QWidget *parent) : QWidget(parent)
{
	pen = QPen(Qt::cyan, 1, Qt::SolidLine);
	brush = QBrush(Qt::green, Qt::SolidPattern);
}

void painter::start_drawing(){
	numbers.clear();
	std::thread tr(&painter::drawing_listen, this);
	drawing.test_and_set();
	
	tr.detach();
}

void painter::set_connect(serial_listener *listene){
	listen = listene;
	return;
}

void painter::drawing_listen(){
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000/100));
		
		if (!listen->new_read)
			continue;
		if (listen->command_n == "output"){
			
			listen->new_read = false;
			int num_ = std::stoi(listen->param_n);
			numbers.push_back(num_);
			if (num_ > max_){
				max_ = num_;
			}
		}
			
		
		update();
	}
	while(drawing.test());
}

void painter::paintEvent(QPaintEvent *event){	
	QPainter p(this);
	p.setPen(pen);
	p.setBrush(brush);

	p.fillRect(rect(), Qt::black);
	
	if (true){
		int margin_m = 50;
		int margin = 30;
		int margin_n = 10;
		float diff_v = (height() - (margin_n + margin));
		float diff_h = (width() - (margin_n + margin_m));
		int mid_l = 6;

		//Diuja el eje y:
		for (int i = 0; i <= steps_v; i++){
			p.drawLine(
				   margin_m-mid_l,
				   height()-margin-(diff_v/steps_v)*i,
				   margin_m,
				   height()-margin-(diff_v/steps_v)*i
				   );
		}

		//Dibuja el eje x:
		for (int i = 0; i <= steps_h; i++){
			p.drawLine(
				   margin_m+(diff_h/steps_h)*i,
				   height()-margin+mid_l,
				   margin_m+(diff_h/steps_h)*i,
				   height()-margin);
		}

		//Cantidad de elementos en el vector numbers
		int size_n = numbers.size();
		//Numero mínimo:
		int min_n = (size_n > max_num)? (size_n - max_num) : 0;
		//Ancho de datos:
		int w_dat = (size_n > max_num)? (max_num) : (size_n);
		
		//Dibuja cada numero desde min_n hasta size_n
		for (int i = min_n + 1; i < size_n; i++){
			p.drawLine(
				   (margin_m) + (diff_h/static_cast<float>(w_dat))*(i-1-min_n),
				   (height() - margin) - (diff_v*numbers[i-1]/(max_ + max_*0.20f)),
				   (margin_m) + (diff_h/static_cast<float>(w_dat))*(i-min_n),
				   (height() - margin) - (diff_v*numbers[i]/(max_ + max_*0.20f)));
		}

		p.drawLine(margin_m, height()-margin, margin_m, margin_n);
		p.drawLine(margin_m, height()-margin, width()-margin_n, height()-margin);
	}
}
