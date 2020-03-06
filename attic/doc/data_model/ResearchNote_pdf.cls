%%---------------------------------------------------------------------------%%
%% ResearchNote.cls
%%
%% OPTIONS:
%%     memo -> gives memo
%%     note -> gives research note (default)
%%     all other options excepted by article
%%
%% COMMANDS:
%%     \memotoc -> tableofcontents blocked by double lines
%%     \classification -> puts classification markers at top and
%%                        bottom of page
%%     \revisionnum -> revision number that goes under the subject
%%     (defaults to one)
%%---------------------------------------------------------------------------%%
%% $Id: ResearchNote_pdf.cls,v 1.2 2009/01/07 23:46:55 tkelley Exp $
%%---------------------------------------------------------------------------%%

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{ResearchNote}
[2003/10/21 LANL Research Note Class, T.M. Evans]

%% OPTIONS
\DeclareOption{memo}{%
  \def\memotitle{\rule{0pt}{21pt}memo\kern.02em randum}}
\DeclareOption{note}{%
  \def\memotitle{\rule{0pt}{21pt}research \kern.02em note}}
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{article}}
\ExecuteOptions{note}
\ProcessOptions

%% This is based on the article class
\LoadClass{article}
\RequirePackage{graphicx}
\RequirePackage{color}
%\RequirePackage{pstcol}
%\RequirePackage{pstricks}

%% Modify the normal text area on the page.
\setlength{\textwidth}{6.5in}
\setlength{\textheight}{8.5in}
\setlength{\oddsidemargin}{0in}
\setlength{\topmargin}{-3\baselineskip}
\setlength{\headheight}{5\baselineskip}
\setlength{\headsep}{18pt}
\setlength{\footskip}{3\baselineskip}

%% New names
\renewcommand{\abstractname}{Executive Summary}

%% Memo tableofcontents
\newcommand{\lines}{%
%  \begin{pspicture}(\textwidth,.25in)
%    \psline(0,0.094in)(\textwidth, 0.094in)
%    \psline(0,.156in)(\textwidth, .156in)
%  \end{pspicture}
}

\newcommand{\memotoc}{%
  \noindent\lines
  \vspace{-3\baselineskip}
  \tableofcontents
  \noindent\lines
}

%% Define a bunch of stuff for the header block
\def\revisionNumber{1}
\def\srd{}
\def\divname{}
\def\grpname{}
\def\fromwho{}
\def\phonenum{}
\def\originatr{}
\def\typst{}
\def\towho{}
\def\through{}
\def\refnum{}
\def\subjectline{}
\def\distrib{}
\def\cyy{}
\def\encl{}
\def\encass{}
\def\attach{}
\def\attachas{}
\def\attachsas{}

\newcommand*{\revisionnum}[1]{\def\revisionNumber{#1}}
\newcommand*{\classification}[1]{\def\srd{#1}}
\newcommand*{\divisionname}[1]{\def\divname{#1}}
\newcommand*{\groupname}[1]{\def\grpname{#1}}
\newcommand*{\fromms}[1]{\def\fromwho{#1}}
\newcommand*{\phone}[1]{\def\phonenum{#1}}
\newcommand*{\originator}[1]{\def\originatr{\uppercase{#1}}}
\newcommand*{\typist}[1]{\def\typst{#1}}
\newcommand*{\toms}[1]{\def\towho{#1}}
\newcommand*{\thru}[1]{\def\through{#1}}
\newcommand*{\refno}[1]{\def\refnum{#1}}
\newcommand*{\subject}[1]{\def\subjectline{#1}}
\newcommand{\distribution}[1]{\def\distrib{#1}}
\newcommand{\cy}[1]{\def\cyy{#1}}
\newcommand{\enc}[1]{\def\encl{#1}}
\newcommand{\encas}{\def\encass{Enc. a/s}}
\newcommand{\attachments}[1]{\def\attach{#1}}
\newcommand{\attachmentas}{\def\attachas{Attachment a/s}}
\newcommand{\attachmentsas}{\def\attachsas{Attachments a/s}}

%% headings (regular pages)
\renewcommand{\ps@headings}{%
  \renewcommand{\@oddhead}{
    %\parbox[b]{1.5in}{\textsl{To \towho}\\ \refnum}
    %\parbox[b]{2.5in}{\textsl{\subjectline}\\ \refnum}
    \parbox[b]{2.5in}{\textsl{DRAFT: FleCSI: Data Requirements}\\ \refnum}
    \hfil
    \parbox[b]{2.0in}{%\centerline{\Large\srd}\vspace{3\baselineskip}
      \centerline{-\textsl{\thepage}-}
      }
    \hfil
    \parbox[b]{1.5in}{\raggedleft\@date}}
  \renewcommand{\@oddfoot}{\centerline{\Large\srd}}%
  \renewcommand{\@evenhead}{}%
  \renewcommand{\@evenfoot}{}%
}

\pagestyle{headings}

%% title page heading
\renewcommand{\ps@empty}{%
  \renewcommand{\@oddhead}{
    \hfil
    \parbox[b]{2.5in}{\centerline{\Large\srd}\vspace{3\baselineskip}
    \centerline{$ $}}
    \hfil}
  \renewcommand{\@oddfoot}{\centerline{\Large\srd}}%
  \renewcommand{\@evenhead}{}%
  \renewcommand{\@evenfoot}{}%
}

%% title page -> opening
\newcommand{\opening}{%
  \thispagestyle{empty}
  \begin{minipage}{2.95in}
    \hspace{-3.4em}
    {\includegraphics[width=2.6in]{logo}}\\
    {\sffamily\bfseries\fontsize{21}{21}\selectfont \memotitle}\\
    {\rmfamily\bfseries\slshape\fontsize{8}{9}\selectfont \divname}\\
    {\rmfamily\itshape\fontsize{8}{9}\selectfont \grpname}
  \end{minipage}
  \hfil
  \begin{minipage}[t]{3.5in}
    \begin{tabular}{rl}
      {\scriptsize To/MS:}&\begin{minipage}[t]{2.75in}
        \towho
      \end{minipage}\\
      {\scriptsize From/MS:}&\begin{minipage}[t]{2.75in}
        \fromwho
      \end{minipage}\\
      {\scriptsize Phone/FAX:}&\phonenum\\
      {\scriptsize Symbol:}&\refnum\\
      {\scriptsize Date:}&\@date
    \end{tabular}
  \end{minipage}

  \par
  \vspace{24pt}
  \noindent\textbf{\large Subject: \subjectline}\\
  \noindent\textbf{\large Revision: \revisionNumber}
  \par
}

%% closing
\newcommand{\closing}{%
  \setlength{\parindent}{0in}
  \par
  \noindent
  \originatr:\typst
  \ifx\attach\@empty\else
  \par
  \noindent
  Attachments:\\
  \attach
  \fi
  \ifx\attachas\@empty\else
  \par
  \noindent
  \attachas
  \fi
  \ifx\attachsas\@empty\else
  \par
  \noindent
  \attachsas
  \fi
  \ifx\encl\@empty\else
  \par
  \noindent
  Enclosures:\\
  \encl
  \fi
  \ifx\encass\@empty\else
  \par
  \noindent
  \encass
  \fi
  \ifx\distrib\@empty\else
  \par
  \noindent
  Distribution:\\
  \distrib
  \fi
  \ifx\cyy\@empty\else
  \par
  \noindent
  Cy:\\
  \cyy
  \fi
}

%%---------------------------------------------------------------------------%%
%% end of ResearchNote.cls3
%%---------------------------------------------------------------------------%%
