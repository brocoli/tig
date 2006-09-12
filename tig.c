#define SIZEOF_REV	41	/* Holds a SHA-1 and an ending NUL */

/* Revision graph */

#define REVGRAPH_INIT	'I'
#define REVGRAPH_MERGE	'M'
#define REVGRAPH_BRANCH	'+'
#define REVGRAPH_COMMIT	'*'
#define REVGRAPH_LINE	'|'

	char id[SIZEOF_REV];	/* Commit SHA1 ID */
LINE(ACKED,	   "    Acked-by",	COLOR_YELLOW,	COLOR_DEFAULT,	0), \
do_scroll_view(struct view *view, int lines)
	assert(view_is_displayed(view));

	do_scroll_view(view, lines);
move_view(struct view *view, enum request request)
	int scroll_steps = 0;
		scroll_steps = steps;
			scroll_steps = -view->offset;
				scroll_steps = view->lines - view->offset - 1;
				if (scroll_steps >= view->height)
					scroll_steps -= view->height - 1;
	}
	if (!view_is_displayed(view)) {
		view->offset += steps;
		view->ops->select(view, &view->line[view->lineno]);
	/* Repaint the old "current" line if we be scrolling */
	if (ABS(steps) < view->height)
		draw_view_line(view, view->lineno - steps - view->offset);
	if (scroll_steps) {
		do_scroll_view(view, scroll_steps);
	}

	/* Draw the current line */
	draw_view_line(view, view->lineno - view->offset);
		do_scroll_view(prev, lines);
		move_view(view, request);
			move_view(view, request);
			if (view_is_displayed(view))
			move_view(view, request);
/* Parse output from git-ls-tree(1):
	enum open_flags flags = display[0] == view ? OPEN_SPLIT : OPEN_DEFAULT;
			char *data = line->data;
	char *text = line->data;

	text += STRING_SIZE("100644 blob ");

	if (line->type == LINE_TREE_FILE) {
		string_ncopy(ref_blob, text, 40);
		/* Also update the blob view's ref, since all there must always
		 * be in sync. */
		string_copy(VIEW(REQ_VIEW_BLOB)->ref, ref_blob);
	} else if (line->type != LINE_TREE_DIR) {
		return;

	string_ncopy(view->ref, text, 40);
 * Revision graph
	char id[SIZEOF_REV];		/* SHA1 ID. */
/* Size of rev graph with no  "padding" columns */
#define SIZEOF_REVITEMS	(SIZEOF_REVGRAPH - (SIZEOF_REVGRAPH / 2))

struct rev_graph {
	struct rev_graph *prev, *next, *parents;
	char rev[SIZEOF_REVITEMS][SIZEOF_REV];
	size_t size;
	struct commit *commit;
	size_t pos;
};

/* Parents of the commit being visualized. */
static struct rev_graph graph_parents[3];

/* The current stack of revisions on the graph. */
static struct rev_graph graph_stacks[3] = {
	{ &graph_stacks[2], &graph_stacks[1], &graph_parents[0] },
	{ &graph_stacks[0], &graph_stacks[2], &graph_parents[1] },
	{ &graph_stacks[1], &graph_stacks[0], &graph_parents[2] },
};

static inline bool
graph_parent_is_merge(struct rev_graph *graph)
{
	return graph->parents->size > 1;
}

static inline void
append_to_rev_graph(struct rev_graph *graph, chtype symbol)
{
	if (graph->commit->graph_size < ARRAY_SIZE(graph->commit->graph) - 1)
		graph->commit->graph[graph->commit->graph_size++] = symbol;
}

static void
done_rev_graph(struct rev_graph *graph)
{
	if (graph_parent_is_merge(graph) &&
	    graph->pos < graph->size - 1 &&
	    graph->next->size == graph->size + graph->parents->size - 1) {
		size_t i = graph->pos + graph->parents->size - 1;

		graph->commit->graph_size = i * 2;
		while (i < graph->next->size - 1) {
			append_to_rev_graph(graph, ' ');
			append_to_rev_graph(graph, '\\');
			i++;
		}
	}

	graph->size = graph->pos = 0;
	graph->commit = NULL;
	memset(graph->parents, 0, sizeof(*graph->parents));
}

static void
push_rev_graph(struct rev_graph *graph, char *parent)
{
	/* Combine duplicate parents lines. */
	if (graph->size > 0 &&
	    !strncmp(graph->rev[graph->size - 1], parent, SIZEOF_REV))
		return;

	if (graph->size < SIZEOF_REVITEMS) {
		string_ncopy(graph->rev[graph->size++], parent, SIZEOF_REV);
	}
}

static chtype
get_rev_graph_symbol(struct rev_graph *graph)
{
	chtype symbol;

	if (graph->parents->size == 0)
		symbol = REVGRAPH_INIT;
	else if (graph->parents->size > 1)
		symbol = REVGRAPH_MERGE;
	else if (graph->pos >= graph->size)
		symbol = REVGRAPH_BRANCH;
	else
		symbol = REVGRAPH_COMMIT;

	return symbol;
}

static void
draw_rev_graph(struct rev_graph *graph)
{
	chtype separator, line;
	chtype symbol = get_rev_graph_symbol(graph);
	size_t i;

	separator = ' ';
	line = REVGRAPH_LINE;

	for (i = 0; i < graph->pos; i++) {
		append_to_rev_graph(graph, line);
		if (graph_parent_is_merge(graph->prev) &&
		    graph->prev->pos == i) {
			separator = '`';
			line = '.';
		}
		append_to_rev_graph(graph, separator);
	}

	/* Place the symbol for this revision. */
	append_to_rev_graph(graph, symbol);

	if (graph->prev->size > graph->size) {
		separator = '\'';
		line = ' ';
	} else {
		separator = ' ';
		line = REVGRAPH_LINE;
	}
	i++;

	for (; i < graph->size; i++) {
		append_to_rev_graph(graph, separator);
		append_to_rev_graph(graph, line);
		if (graph_parent_is_merge(graph->prev)) {
			if (i < graph->prev->pos + graph->parents->size) {
				separator = '`';
				line = '.';
			}
		}
		if (graph->prev->size > graph->size) {
			separator = '/';
			line = ' ';
		}
	}

	if (graph->prev->size > graph->size) {
		append_to_rev_graph(graph, separator);
		if (line != ' ')
			append_to_rev_graph(graph, line);
	}
}

void
update_rev_graph(struct rev_graph *graph)
{
	size_t i;

	/* First, traverse all lines of revisions up to the active one. */
	for (graph->pos = 0; graph->pos < graph->size; graph->pos++) {
		if (!strcmp(graph->rev[graph->pos], graph->commit->id))
			break;

		push_rev_graph(graph->next, graph->rev[graph->pos]);
	}

	/* Interleave the new revision parent(s). */
	for (i = 0; i < graph->parents->size; i++)
		push_rev_graph(graph->next, graph->parents->rev[i]);

	/* Lastly, put any remaining revisions. */
	for (i = graph->pos + 1; i < graph->size; i++)
		push_rev_graph(graph->next, graph->rev[i]);

	draw_rev_graph(graph);
	done_rev_graph(graph->prev);
}


/*
 * Main view backend
 */

	static struct rev_graph *graph = graph_stacks;
		graph->commit = commit;
		break;

	case LINE_PARENT:
		if (commit) {
			line += STRING_SIZE("parent ");
			push_rev_graph(graph->parents, line);
		}
		update_rev_graph(graph);
		graph = graph->next;
