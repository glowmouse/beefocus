cd code_docs

if [ -f "index.html" ]; then

  git add --all
  git commit -m "Deploy code docs to GitHub pages"
  git push --force

fi

