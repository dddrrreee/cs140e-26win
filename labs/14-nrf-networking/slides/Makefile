NAME = 14-nrf-notes

all:
	marp  --allow-local-files  --theme narrow-bullets.css --html $(NAME).md -o $(NAME).pdf
	evince $(NAME).pdf

clean:
	rm -f $(NAME).pdf *~
