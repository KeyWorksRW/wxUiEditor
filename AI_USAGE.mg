# How this project is maintained

## AI-assisted development

This project is maintained with a heavily customized AI-assisted workflow.
I am writing this note to be upfront about that, because undisclosed AI use in open source is a legitimate complaint,
and because the honest version of this story is more defensible than whatever rumors would come up with.

## What the workflow looks like

I use a private development harness with custom agent tooling and custom agent definitions.
I am not publishing the harness or the agents because they are tightly coupled to my personal environment and would be
useless — and misleading as a template — for anyone else.
What follows describes what they do, not what they are.

At a high level, I act as director and overseer rather than as a typist:

- I define what needs to change and why.
  The specification, the scope, and the acceptance criteria are mine.
- Custom agents propose and implement changes against that specification.

## On code review

I do not do a line-by-line review of the generated code.
Instead, I use a triple agent code review process -- two different agents with completely different approaches to
reviewing code, and a parent agent that takes the reviews and does a thorough review of the reported issues to determine
if they are valid or not.
Any issues found are then presented to me to review.
It’s time consuming and expensive, but I have found it to be much more accurate in finding issues then my reviewing each
line of ai-generated code.

## Why I don’t accept contributors

Currently this project accepts no external contributions.
This is not about the quality of outside work; it is that my AI-driven cycle is fast enough that review bandwidth for
external PRs would become a serious bottleneck, and I would rather the project move quickly than be nominally open.
If that changes, this document will change with it.

# Contributing

Issues are *very* welcome! Feature requests and bug reports are still the best way to influence the project.
