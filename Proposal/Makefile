all:	proposal.tex 
	latex proposal
	bibtex proposal
	latex proposal
	pdflatex proposal

clean: 
	${RM} *.dvi *.log *.aux *.blg *.bbl paper.ps paper.ps paper.pdf

